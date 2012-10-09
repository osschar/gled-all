// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdMonSucker.h"
#include "XrdFileCloseReporter.h"
#include "Glasses/UdpPacketSource.h"
#include "Glasses/ZHashList.h"
#include "Glasses/ZLog.h"
#include "XrdMonSucker.c7"

#include "XrdDomain.h"
#include "XrdServer.h"
#include "XrdUser.h"
#include "XrdFile.h"

#include "Stones/SUdpPacket.h"
#include "Gled/GThread.h"

#include "XrdMon/XrdXrootdMonData.h"

#include <cerrno>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

namespace
{
  const Double_t One_MB = 1024 * 1024;
}

// XrdMonSucker

//______________________________________________________________________________
//
// Enabling trace reports for XrdUser matching those regexps:
//   mTraceDN, mTraceHost, mTraceDomain (clent, not server)
// The match is done at login time.

ClassImp(XrdMonSucker);

//==============================================================================

void XrdMonSucker::_init()
{
  mSuckPort  = 9929;

  mUserKeepSec = 300;
  mUserDeadSec = 86400;
  mServDeadSec = 86400;
  mServIdentSec = 300;
  mServIdentCnt = 5;

  mPacketCount = mSeqIdFailCount = 0;

  mSuckerThread = 0;
  mLastOldUserCheck  = mLastDeadUserCheck  =
  mLastDeadServCheck = mLastIdentServCheck = GTime(GTime::I_Never);

  bTraceAllNull = true;
}

XrdMonSucker::XrdMonSucker(const Text_t* n, const Text_t* t) :
  ZNameMap(n, t)
{
  _init();
  SetElementFID(XrdDomain::FID());
}

XrdMonSucker::~XrdMonSucker()
{}

void XrdMonSucker::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  if (mOpenFiles == 0)
  {
    assign_link<ZHashList>(mOpenFiles, FID(), "OpenFiles");
    mOpenFiles->SetElementFID(XrdFile::FID());
  }
  if (mFCReporters == 0)
  {
    assign_link<ZHashList>(mFCReporters, FID(), "FileCloseReporters");
    mFCReporters->SetElementFID(XrdFileCloseReporter::FID());
    mFCReporters->SetMIRActive(false);
  }
}

//==============================================================================

void XrdMonSucker::AddFileCloseReporter(XrdFileCloseReporter* fcr)
{
  mFCReporters->Add(fcr);
}

void XrdMonSucker::RemoveFileCloseReporter(XrdFileCloseReporter* fcr)
{
  mFCReporters->Remove(fcr);
}

//==============================================================================

void XrdMonSucker::on_file_open(XrdFile* file)
{
  auto_ptr<ZMIR> mir( mOpenFiles->S_Add(file) );
  mSaturn->ShootMIR(mir);
}

void XrdMonSucker::on_file_close(XrdFile* file, XrdUser* user, XrdServer* server)
{
  Stepper<XrdFileCloseReporter> stepper(*mFCReporters);
  while (stepper.step())
  {
    stepper->FileClosed(file, user, server);
  }

  {
    auto_ptr<ZMIR> mir( mOpenFiles->S_Remove(file) );
    mSaturn->ShootMIR(mir);
  }
}

//------------------------------------------------------------------------------

void XrdMonSucker::disconnect_user_and_close_open_files(XrdUser* user, XrdServer* server,
                                                        const GTime& time)
{
  static const Exc_t _eh("XrdMonSucker::disconnect_user_and_close_open_files ");

  {
    GLensReadHolder _lck(user);
    user->SetDisconnectTime(time);
  }

  list<XrdFile*> open_files;
  user->CopyListByGlass<XrdFile>(open_files);
  for (list<XrdFile*>::iterator fi = open_files.begin(); fi != open_files.end(); ++fi)
  {
    XrdFile *file = *fi;
    Bool_t closed = false;
    {
      GLensReadHolder _lck(file);
      if (file->IsOpen())
      {
        file->SetCloseTime(time);
        closed = true;
      }
    }
    if (closed)
    {
      {
        GLensReadHolder _lck(server);
	try
	{
	  server->RemoveFile(file);
	}
	catch (Exc_t exc)
	{
	  if (*mLog)
	    mLog->Put(ZLog::L_Error, _eh, exc);
	}
      }
      on_file_close(file, user, server);
    }
  }
  {
    GLensWriteHolder _lck(server);
    try
    {
      server->DisconnectUser(user);
    }
    catch (Exc_t exc)
    {
      if (*mLog)
	mLog->Put(ZLog::L_Error, _eh, exc);
    }
  }
}

//------------------------------------------------------------------------------

void XrdMonSucker::disconnect_server(XrdServer* server, XrdDomain *domain,
				     const GTime& time)
{
  {
    GMutexHolder _lck(m_xrd_servers_mutex);
    m_xrd_servers.erase(server->m_server_id);
  }
  {
    list<XrdUser*> users;
    server->CopyListByGlass<XrdUser>(users);
    for (list<XrdUser*>::iterator ui = users.begin(); ui != users.end(); ++ui)
    {
      XrdUser *user = *ui;
      disconnect_user_and_close_open_files(user, server, time);
    }
  }
  server->ClearPrevUserMap();
  mSaturn->ShootMIR( mQueen->S_RemoveLenses(server->GetPrevUsers()) );
  mSaturn->ShootMIR( domain->S_RemoveAll(server) );
}

//==============================================================================

void XrdMonSucker::Suck()
{
  static const Exc_t _eh("XrdMonSucker::Suck ");

  TPMERegexp ip4addr_re ("(\\d+\\.\\d+\\.\\d+)\\.(\\d+)", "o");
  TPMERegexp username_re("(\\w+)\\.(\\d+):(\\d+)@(.+)", "o");
  TPMERegexp hostname_re("([^\\.]+)\\.(.*)", "o");
  TPMERegexp authinfo_re("^&p=(.*)&n=(.*)&h=(.*)&o=(.*)&r=(.*)&g=(.*)&m=(.*)$", "o");
  TPMERegexp authxxxx_re("^&p=(.*)&n=(.*)&h=(.*)&o=(.*)&r=(.*)$", "o");

  TPMERegexp redir_re   ("(.*?):(.*)", "o");

  while (true)
  {
    SUdpPacket *p = mUdpQueue.PopFront();

    GThread::CancelDisabler _cd;

    GTime recv_time(p->mRecvTime);

    {
      GLensReadHolder _lck(this);
      if (++mPacketCount % 1000 == 0)
        Stamp(FID());
    }

    XrdXrootdMonHeader *xmh = (XrdXrootdMonHeader*) p->mBuff;
    Char_t   code = xmh->code;
    UChar_t  pseq = xmh->pseq;
    UShort_t plen = ntohs(xmh->plen);
    Int_t    stod = ntohl(xmh->stod);
    UInt_t   in4a = p->Ip4AsUInt(); // Kept in net order
    UShort_t port = p->mPort;

    SXrdServerId xsid(in4a, stod, port);
    xrd_hash_i   xshi;
    bool         server_not_known;
    {
      GMutexHolder _lck(m_xrd_servers_mutex);
      xshi = m_xrd_servers.find(xsid);
      server_not_known = (xshi == m_xrd_servers.end());
    }

    ZLog::Helper log(*mLog, recv_time, ZLog::L_Info, _eh);

    XrdServer *server = 0;
    XrdDomain *domain = 0;

    if (server_not_known)
    {
      sockaddr_in  sa4;
      sockaddr_in6 sa6;
      sockaddr    *sa = 0;
      socklen_t    sl;
      if (p->mAddrLen == 4)
      {
	sa4.sin_family = AF_INET;
	memcpy(&sa4.sin_addr.s_addr, p->mAddr, p->mAddrLen);
	sa = (sockaddr*) &sa4;
	sl = sizeof(sa4);
      }
      else
      {
	sa6.sin6_family = AF_INET6;
	memcpy(sa6.sin6_addr.s6_addr, p->mAddr, p->mAddrLen);
	sa = (sockaddr*) &sa6;
	sl = sizeof(sa6);
      }

      Char_t   hn_buf[64];
      getnameinfo((sockaddr*) sa, sl, hn_buf, 64, 0, 0, NI_DGRAM);

      TString fqhn(hn_buf);
      fqhn.ToLower();
      if (hostname_re.Match(fqhn) != 3)
      {
        char *foo = (char*) &in4a;
        log.Form(ZLog::L_Error, "New server NS lookup problem: %hhu.%hhu.%hhu.%hhu:%hu, fqdn='%s'.",
                 foo[0], foo[1], foo[2], foo[3], port, hn_buf);
	continue;
      }

      log.Form(ZLog::L_Message, "New server: %s.%s:%hu stod=%d",
	       hostname_re[1].Data(), hostname_re[2].Data(), port, stod);

      server = new XrdServer(GForm("%s.%s : %d : %hu", hostname_re[1].Data(), hostname_re[2].Data(), stod, port),
                             "", hostname_re[1], hostname_re[2], GTime(stod));
      server->m_server_id = xsid;

      domain = static_cast<XrdDomain*>(GetElementByName(server->GetDomain()));
      if (!domain)
      {
        domain = new XrdDomain(server->GetDomain());
        // ZQueen::CheckIn() does write lock.
        mQueen->CheckIn(domain);
        {
          GLensWriteHolder _lck(this);
          Add(domain);
        }
      }

      // ZQueen::CheckIn() does write lock.
      mQueen->CheckIn(server);
      {
        GLensWriteHolder _lck(domain);
        domain->Add(server);
      }
      {
        GMutexHolder _lck(m_xrd_servers_mutex);
        xshi = m_xrd_servers.insert(make_pair(xsid, server)).first;
      }
    }
    else
    {
      server = xshi->second;
      domain = static_cast<XrdDomain*>(GetElementByName(server->GetDomain()));
    }

    {
      GLensReadHolder _lck(server);
      server->IncPacketCount();
      server->SetLastMsgTime(recv_time);
    }
    {
      GLensReadHolder _lck(domain);
      domain->IncPacketCount();
    }

    // Check sequence id. No remedy attempted.
    if (code == 'u' || code == 'd' || code == 't' || code == 'i' || code == 'r')
    {
      if (server->IsSrvSeqInited())
      {
	UChar_t srv_seq = server->IncAndGetSrvSeq();
	if (pseq != srv_seq)
	{
	  log.Form(ZLog::L_Warning, "Sequence-id mismatch at '%s' srv=%hhu, msg=%hhu; code=%c. Ignoring.",
		   server->GetName(), srv_seq, pseq, code);
	  server->InitSrvSeq(pseq);
          {
            GLensReadHolder _lck(server);
            server->IncSeqIdFailCount();
          }
          {
            GLensReadHolder _lck(domain);
            domain->IncSeqIdFailCount();
          }
          {
            GLensReadHolder _lck(this);
            ++mSeqIdFailCount;
            Stamp(FID());
          }
	}
      }
      else
      {
	server->InitSrvSeq(pseq);
      }
    }

    // Check length of message .vs. length claimed by xrd.
    if (p->mBuffLen != plen)
    {
      log.Form(ZLog::L_Warning, "Message size mismatch: got %zd, xrd-len=%hu.", p->mBuffLen, plen);
      // This means either our buf-size is too small or the other guy is pushing it.
      // Should probably stop reporting errors from this IP.
      // XXXX Do additional checks in here about buf-size, got less, etc.
      continue;
    }

    if (code != 't' && code != 'r')
    {
      TString msg;
      msg.Form("Message from %s.%s:%hu, c=%c, seq=%hhu, len=%hu",
               server->GetHost(), server->GetDomain(), port,
               xmh->code, pseq, plen);

      XrdXrootdMonMap *xmm     = (XrdXrootdMonMap*) p->mBuff;
      Int_t            dict_id = ntohl(xmm->dictid);

      (p->mBuff)[plen] = 0; // 0-terminate the buffer at packet length.

      char *prim = xmm->info;
      char *sec  = strstr(prim, "\n");
      if (sec) {
	*(sec++) = 0;
      }

      if (code == 'u')
      {
	msg += TString::Format("\n\tUser map -- id=%d, uname=%s", dict_id, prim);
	TString uname(prim), host, domain;
        Bool_t  numeric_host = false;
        {
          if (username_re.Match(uname) != 5)
          {
            msg += " ... parse error.";
            log.Put(ZLog::L_Error, msg);
            continue;
          }

          if (ip4addr_re.Match(username_re[4]) == 3)
          {
            // Numeric ip, assume private subnet (event though we really don't
            // know as this is blindly taken from client and subnet can
            // actually be anywhere in the world).
            msg += TString::Format("@%s", server->GetDomain());
            host = ip4addr_re[0];
            domain = server->RefDomain();
            numeric_host = true;
          }
          else if (hostname_re.Match(username_re[4]) == 3)
          {
            // Domain given
            host   = hostname_re[1];
            domain = hostname_re[2];
          }
          else
          {
            // No domain, same as XrdServer
            msg += TString::Format(".%s", server->GetDomain());
            host   = username_re[4];
            domain = server->RefDomain();
          }

          if (username_re[1] == mNagiosUser && host.BeginsWith(mNagiosHost) && domain.BeginsWith(mNagiosDomain))
          {
            // msg += TString::Format(" ... it is nagios, skipping it.\n");
            // cout << msg;
            continue;
          }

          // Go figure, in fall 2011 MIT was sending two user-map messages.
	  // The problem with this is that file-map uses user-name to identify
          // user ... and obviously the files were assigned to the wrong one +
          // one session remained open forever.
          {
            XrdUser *xu = server->FindUser(uname);
            if (xu != 0)
            {
              msg += "\n\tUsername was already taken -- deleting old user!";
              disconnect_user_and_close_open_files(xu, server, recv_time);
            }
          }

          if (server->ExistsUserDictId(dict_id))
          {
            msg += "\n\tUser dict_id already taken ... this session will not be tracked.";
            log.Put(ZLog::L_Warning, msg);
            continue;
          }
        }

        XrdUser *user = 0;
        try
        {
          TString  dn;
          {
            TString     group;
            TPMERegexp *a_rep = 0;
            if (authinfo_re.Match(sec) == 8)
            {
              group =  authinfo_re[6];
              dn    =  authinfo_re[7];
              a_rep = &authinfo_re;
            }
            else if (authxxxx_re.Match(sec) == 6)
            {
              group =  "<unknown>";
              dn    =  "<unknown>";
              a_rep = &authxxxx_re;
            }
            else
            {
              msg += GForm("\n\tUnparsable auth-info: '%s'", sec);
            }

            if (a_rep)
            {
              TPMERegexp &a_re = *a_rep;

              msg += GForm("\n\tDN=%s, VO=%s, Role=%s, Group=%s",
                           dn.Data(), a_re[4].Data(), a_re[5].Data(), group.Data());

              user = new XrdUser(uname, "", dn, a_re[4], a_re[5], group,
                                 a_re[2], host, domain, numeric_host, recv_time);
            }
            else
            {
              user = new XrdUser(uname, "", "", "", "", "",
                                 "", host, domain, numeric_host, recv_time);
            }
            // ZQueen::CheckIn() does write lock.
            mQueen->CheckIn(user);
          }

          {
            GLensWriteHolder _lck(server);
            server->AddUser(user, dict_id);
          }
          {
            GLensWriteHolder _lck(user);
            user->SetServer(server);

            if ( ! bTraceAllNull && mTraceDN_RE.Match(dn) &&
                 mTraceHost_RE.Match(host) && mTraceDomain_RE.Match(domain))
            {
              user->SetTraceMon(true);
            }          
          }
        }
        catch (Exc_t exc)
        {
          msg += "\n\tException caught while instantiating XrdUser:\n\t" + exc;
          log.Put(ZLog::L_Error, msg);
          continue;
        }

	// XXX Eventually ... grep / create GridUser.
      }
      else if (code == 'd')
      {
        TString uname(prim);
        TString path (sec);
	msg += TString::Format("\n\tPath map -- id=%d, uname=%s path=%s",
                               dict_id, uname.Data(), path.Data());
	XrdUser *user = server->FindUser(uname);
	if (user)
	{
	  if (server->ExistsFileDictId(dict_id))
	  {
	    msg += "\n\tFile dict_id already taken ... this file will not be tracked.";
            log.Put(ZLog::L_Warning, msg);
	    continue;
	  }

	  // create XrdFile
          try
          {
            XrdFile *file = new XrdFile(path);
            mQueen->CheckIn(file);
            {
              GLensWriteHolder _lck(user);
              user->AddFile(file);
              user->SetLastMsgTime(recv_time);
            }
            {
              GLensWriteHolder _lck(file);
              file->SetUser(user);
              file->SetOpenTime(recv_time);
              file->SetLastMsgTime(recv_time);
            }
            {
              GLensWriteHolder _lck(server);
              server->AddFile(file, dict_id);
            }

            on_file_open(file);
          }
          catch (Exc_t exc)
          {
            msg += "\n\tException caught while instantiating XrdFile:\n\t" + exc;
            log.Put(ZLog::L_Error, msg);
            continue;
          }
	}
	else
	{
          if ( ! uname.BeginsWith(mNagiosUser))
          {
            msg += "\n\tUser not found ... skipping.";
            log.Put(ZLog::L_Warning, msg);
          }
	  continue;
	}
      }
      else if (code == 'i')
      {
        TString uname(prim);
        TString info (sec);
	msg += TString::Format("\n\tInfo map -- id=%d, uname=%s info=%s",
                               dict_id, uname.Data(), info.Data());

	XrdUser *user = server->FindUser(uname);
	if (user)
        {
          GLensWriteHolder _lck(user);
          user->AppendAppInfo(info);
          user->SetLastMsgTime(recv_time);
        }
        else
        {
          msg += "\n\tUser not found ... skipping.";
          log.Put(ZLog::L_Warning, msg);
        }
      }
      else if (code == '=')
      {
	GLensWriteHolder _lck(server);
	server->UpdateSrvIdTime(recv_time);
	msg += TString::Format("\n\tServerId -- uname=%s other=%s", prim, sec);
      }
      else
      {
	msg += TString::Format("\n\tOther %c -- id=%u, uname=%s other=%s", code, dict_id, prim, sec);
      }

      log.Put(msg);
    }
    else if (code == 't') // this is a trace message
    {
      struct local_cache
      {
	XrdServer        *fSrv;
	XrdXrootdMonBuff *fXmb;

	XrdFile          *fFile;
	Int_t             fDictId;

	Int_t             fTi, fTiWEnd; // time-idx, time-idx-window-end
	const Int_t       fN;
	GTime             fTime;
	Double_t          fTimeStep;

	local_cache(XrdServer* s, XrdXrootdMonBuff* b, Int_t plen) :
	  fSrv(s), fXmb(b),
	  fFile(0), fDictId(0),
	  fTi(-1), fTiWEnd(-1),
	  fN((plen - sizeof(XrdXrootdMonHeader)) / sizeof(XrdXrootdMonTrace))
	{}

	XrdXrootdMonTrace& trace()        { return fXmb->info[fTi]; }
	XrdXrootdMonTrace& trace(int idx) { return fXmb->info[idx]; }

	UChar_t trace_type()        { return trace().arg0.id[0]; }
	UChar_t trace_type(int idx) { return trace(idx).arg0.id[0]; }

        Int_t full_delta_time()
        {
          return ntohl(trace(fN-1).arg1.Window) - ntohl(trace(0).arg2.Window);
        }

	Bool_t next(TString& log_msg, Bool_t verbose=false)
	{
	  if (++fTi < fN)
	  {
	    if (trace_type() == XROOTD_MON_WINDOW)
	    {
	      fTiWEnd = fTi + 1;
	      while (fTiWEnd < fN && trace(fTiWEnd).arg0.id[0] != XROOTD_MON_WINDOW)
		++fTiWEnd;
	      if (fTiWEnd >= fN)
		return false;

	      Int_t n_div = fTiWEnd - fTi - 2;
	      fTime = GTime(ntohl(trace().arg2.Window));
	      fTimeStep = (n_div >= 1) ?
		(GTime(ntohl(trace(fTiWEnd).arg1.Window)) - fTime).ToDouble() / n_div :
		0;

              if (verbose)
                log_msg += GForm("\n\tWindow iB=%2d iE=%2d N=%2d delta_t=%f -- start %s.",
                                 fTi, fTiWEnd, fTiWEnd-fTi-1, fTimeStep, fTime.ToDateTimeLocal(false).Data());

	      ++fTi;
	    }
	    else
	    {
	      fTime += fTimeStep;
	    }
	    return true;
	  }
	  return false;
	}

	XrdFile* update(Int_t newid)
	{
	  if (newid != fDictId)
	  {
	    fFile   = fSrv->FindFile(newid);
	    fDictId = newid;
	  }
	  return fFile;
	}

	const char* trace_type_name()
	{
	  const char *c;
	  switch (trace_type())
	  {
	    case XROOTD_MON_APPID:  c = "apm"; break;
	    case XROOTD_MON_CLOSE:  c = "cls"; break;
	    case XROOTD_MON_DISC:   c = "dis"; break;
	    case XROOTD_MON_OPEN:   c = "opn"; break;
	    case XROOTD_MON_WINDOW: c = "win"; break;
	    default:                c = "rdw"; break;
	  }
	  return c;
	}

        XrdUser* find_user()
        {
          // This is called at the beginning of message processing to get the
          // user and to determine if we need to dump debug-level information.
          // This definitely does not work as intended when servers are not
          // configured to send io traces.
          // Ah, this is also used to set the time of last message on a user.
          for (Int_t i = 1; i < fN; ++i)
          {
            XrdXrootdMonTrace &xmt = trace(i);
            UChar_t tt = trace_type(i);
            if (tt <= 0x7F || tt == XROOTD_MON_OPEN || tt == XROOTD_MON_CLOSE)
            {
              update(ntohl(xmt.arg2.dictid));
              if (fFile) return fFile->GetUser();
            }
            else if (tt == XROOTD_MON_DISC)
            {
              return fSrv->FindUser(ntohl(xmt.arg2.dictid));
            }
          }
          return 0;
        }
      };

      local_cache lc(server, (XrdXrootdMonBuff*) p->mBuff, plen);

      XrdUser *us = lc.find_user();
      Bool_t  vrb = us && us->GetTraceMon();
      TString msg, msg_vrb;

      while (lc.next(msg_vrb, vrb))
      {
	Int_t ti = lc.fTi;
	XrdXrootdMonTrace &xmt = lc.trace();
	UChar_t       tt  = lc.trace_type();
	const Char_t *ttn = lc.trace_type_name();

        // if (vrb) msg_vrb += GForm("\n\t%-2d: %hhx %s", ti, tt, ttn);

        XrdFile *file = 0;

        if (tt <= 0x7F)
        {
	  Int_t dict_id = ntohl(xmt.arg2.dictid);
          file = lc.update(dict_id);
          if (file)
          {
	    us = file->GetUser();
            // Int_t rwoff = ntohl(arg.arg0.val);
            Int_t rwlen = ntohl(xmt.arg1.buflen);
            GLensReadHolder _lck(file);
            if (rwlen >= 0)
            {
              file->AddReadSample ( rwlen / One_MB);
            }
            else
            {
              file->AddWriteSample(-rwlen / One_MB);
            }
            file->SetLastMsgTime(lc.fTime);
          }
        }
        else if (tt == XROOTD_MON_READV)
        {
	  Int_t dict_id = ntohl(xmt.arg2.dictid);
          file = lc.update(dict_id);
          if (file)
          {
	    us = file->GetUser();
            Int_t rlen = ntohl(xmt.arg1.buflen);
            Int_t nels = ntohs(xmt.arg0.sVal[1]);
            // Not processed: vcnt and vseq (for multi file read)
            GLensReadHolder _lck(file);
            file->AddVecReadSample(rlen / One_MB, nels);
            file->SetLastMsgTime(lc.fTime);
          }
        }
	else if (tt == XROOTD_MON_OPEN || tt == XROOTD_MON_CLOSE)
	{
	  Int_t dict_id = ntohl(xmt.arg2.dictid);
	  file = lc.update(dict_id);
          if (vrb) msg_vrb += GForm("\n\t%2d: %s, file='%s'", ti, ttn, file ? file->GetName() : "<nil>");
          if (file)
          {
	    us = file->GetUser();
            if (tt == XROOTD_MON_OPEN)
            {
              msg += GForm("\n\tOpen file='%s'", file ? file->GetName() : "<nil>");
              union { Long64_t val; UChar_t id[8]; } jebo;
              jebo.val = xmt.arg0.val;
              jebo.id[0] = 0;

	      GLensReadHolder _lck(file);
              file->SetSizeMB(net2host(jebo.val) / One_MB);
	      // This should, in principle, be always true ... but some
	      // pre-3.1 xrootds can get this screwed up pretty badly.
	      if (lc.fTime < file->RefOpenTime())
	      {
		file->SetOpenTime(lc.fTime);
	      }
	      file->SetLastMsgTime(lc.fTime);
            }
            else
            {
	      {
		GLensReadHolder _lck(file);
		file->SetLastMsgTime(lc.fTime);
		ULong64_t x;
		x = ntohl(xmt.arg0.rTot[1]);
		x <<= xmt.arg0.id[1];
		file->SetRTotalMB(x / One_MB);
		x = ntohl(xmt.arg1.wTot);
		x <<= xmt.arg0.id[2];
		file->SetWTotalMB(x / One_MB);
		file->SetCloseTime(lc.fTime);
	      }
              msg += GForm("\n\tClose file='%s'", file ? file->GetName() : "<nil>");
	      {
		GLensReadHolder _lck(server);
		try
		{
		  server->RemoveFile(file);
		}
		catch (Exc_t exc)
		{
		  if (*mLog)
		    mLog->Put(ZLog::L_Error, _eh, exc);
		}
	      }
              on_file_close(file, us, server);
            }
          }
	}
	else if (tt == XROOTD_MON_DISC)
	{
	  Int_t    dict_id        = ntohl(xmt.arg2.dictid);
	  XrdUser *us_from_server = server->FindUser(dict_id);
	  if (us != us_from_server)
	  {
	    log.Form(ZLog::L_Warning, _eh + "us != us_from_server: us=%p ('%s'), us_from_server=%p ('%s')",
                     us,             us ? us->GetName() : "",
                     us_from_server, us_from_server ? us_from_server->GetName() : "");
	    if (us_from_server) {
	      us = us_from_server;
	    }
	  }

          bool disconn_p = true;
          TString extra;
          if (xmt.arg0.id[1] & XROOTD_MON_FORCED) {
            extra += "(forced)";
          }
          if (xmt.arg0.id[1] & XROOTD_MON_BOUNDP) {
            disconn_p = false;
            extra += "(bound-path)";
          }

          msg += GForm("\n\tDisconnect%s user='%s'", extra.Data(), us ? us->GetName() : "<nil>");

	  if (vrb) msg_vrb += GForm("\n\t%2d: %s%s, user=%s", ti, ttn, extra.Data(), us ? us->GetName() : "<nil>");
	  if (disconn_p && us)
	  {
            disconnect_user_and_close_open_files(us, server, lc.fTime);
	  }
	}
      } // while trace entries

      if ( ! msg.IsNull() || vrb)
      {
        TString txt;
        txt.Form("Trace from %s.%s:%hu, user='%s', N=%d, dt=%d, seq=%hhu, len=%hu.",
                 server->GetHost(), server->GetDomain(), port, us ? us->GetName() : "<nil>",
                 lc.fN, lc.full_delta_time(), pseq, plen);
        txt += msg;
        if (vrb) txt += msg_vrb;
        log.Put(txt);
      }

      if (us)
      {
        GLensReadHolder _lck(us);
        us->SetLastMsgTime(lc.fTime);
      }

    } // else -- trace message handling
    else if (code == 'r')
    {
      // XXXX Check for 'r' records, dump basic info to see what else to check / account for.
      //  - 'r' probably has the pseq but it will be coming from a master /
      //    redirector which I never tried before.
      //     . enable on xrootd.t2 - report to some other port not to screw up
      //       production system;
      //     . have some printouts

      XrdXrootdMonBurr *rb = (XrdXrootdMonBurr*) p->mBuff;

      TString txt;
      txt.Form("Redirect trace from %s.%s:%hu, seq=%hhu, len=%hu.",
               server->GetHost(), server->GetDomain(), port, pseq, plen);
      log.Put(txt);

      // XXXX In progress ... redirect message processing.
      if (*mRedirectLog)
      {
        ZLog::Helper rlog(*mRedirectLog, GTime::ApproximateTime(), ZLog::L_Message, "");

        int rb_to_read = plen - sizeof(XrdXrootdMonHeader) - sizeof(kXR_int64);
        int i          = 0;
        // int prev_win   = 0;
        XrdXrootdMonRedir *rr = rb->info;
        while (rb_to_read > 0)
        {
          int len = 8 * (rr->arg0.Dent + 1);

          txt += TString::Format("\n  %3d - 0x%02hhx len=%3d: ",
                                 i, rr->arg0.Type, len);

          if (rr->arg0.Type == 0) // time window
          {
            txt += TString::Format("window prev_len=%d, start=%d",
                                   ntohl(rr->arg0.Window), ntohl(rr->arg1.Window));
          }
          else
          {
            UInt_t uid = ntohl(rr->arg1.dictid);
            XrdUser *user = server->FindUserOrPrevUser(uid);

            redir_re.Match((const char*)(&rr->arg1.dictid) + 4);
            TString redir_host = redir_re[1];
            TString redir_lfn  = redir_re[2];

            txt += TString::Format("uid=%u, %s\n        %s:%s",
                                   uid, user ? user->GetName() : "<unknown>",
                                   redir_host.Data(), redir_lfn.Data());

            // Should get proper time

            TString client_host, uname, urealname;
            if (user)
            {
              client_host.Form("%s:%s", user->GetFromHost(), user->GetFromDomain());
              uname     = user->RefName();
              urealname = user->RefRealName();
            }

            rlog.Form("|| %s.%s | %s | %s | %s | %s | %s ||",
                      server->GetHost(), server->GetDomain(),
                      client_host.Data(),
                      uname.Data(),
                      urealname.Data(),
                      redir_lfn.Data(),
                      redir_host.Data());
          }

          rr = (XrdXrootdMonRedir*) ((char*) rr + len);
          rb_to_read -= len;
          ++i;
        }
        rlog.Put(ZLog::L_Info, txt);
      }
    }

    p->DecRefCount();
  } // while (true) main loop
}

//==============================================================================

void* XrdMonSucker::tl_Suck(XrdMonSucker* s)
{
  GThread *thr = GThread::Self();
  s->mSaturn->register_detached_thread(s, thr);
  thr->CleanupPush((GThread_cu_foo) cu_Suck, s);

  {
    GLensReadHolder _lck(s);
    s->bSuckerRunning = true;
    s->Stamp(s->FID());
  }

  s->mSource->RegisterConsumer(&s->mUdpQueue);
  s->Suck();

  return 0;
}

void XrdMonSucker::StartSucker()
{
  static const Exc_t _eh("XrdMonSucker::StartSucker ");

  {
    GLensReadHolder _lck(this);
    if (mSuckerThread)
      throw _eh + "already running.";

    mSuckerThread  = new GThread("XrdMonSucker-Sucker",
                                 (GThread_foo) tl_Suck, this,
                                 false, true);

    mCheckerThread = new GThread("XrdMonSucker-Checker",
                                 (GThread_foo) tl_Check, this,
                                 false, false);
  }
  mLastOldUserCheck  = mLastDeadUserCheck  =
  mLastDeadServCheck = mLastIdentServCheck = GTime::ApproximateTime();

  mSuckerThread->SetNice(0);
  mSuckerThread->Spawn();

  mCheckerThread->SetNice(20);
  mCheckerThread->Spawn();
}

void XrdMonSucker::cu_Suck(XrdMonSucker* s)
{
  s->mSaturn->unregister_detached_thread(s, GThread::Self());

  s->mSource->UnregisterConsumer(&s->mUdpQueue);

  s->mCheckerThread->Cancel();
  s->mCheckerThread->Join();
  delete s->mCheckerThread;

  {
    GLensReadHolder _lck(s);
    s->mSuckerThread = 0;
    s->mCheckerThread = 0;
    s->bSuckerRunning = false;
    s->Stamp(s->FID());
  }
}

void XrdMonSucker::StopSucker()
{
  static const Exc_t _eh("XrdMonSucker::StopSucker ");

  GThread *thr = 0;
  {
    GLensReadHolder _lck(this);
    if ( ! GThread::IsValidPtr(mSuckerThread))
      throw _eh + "not running.";
    thr = mSuckerThread;
    GThread::InvalidatePtr(mSuckerThread);
  }
  thr->ClearDetachOnExit();
  thr->Cancel();
  thr->Join();
  delete thr;
}

//==============================================================================

void* XrdMonSucker::tl_Check(XrdMonSucker* s)
{
  s->Check();
  return 0;
}

void XrdMonSucker::Check()
{
  static const Exc_t _eh("XrdMonSucker::Check ");

  while (true)
  {
    GTime now = GTime::ApproximateTime();

    {
      GThread::CancelDisabler _cd;

      bool stamp_p = false;
      GLensReadHolder _lck(this);
      if ((now - mLastOldUserCheck).GetSec() > mUserKeepSec)
      {
        mSaturn->ShootMIR( S_CleanUpOldUsers() );
        mLastOldUserCheck = now;
        stamp_p = true;
      }
      if ((now - mLastDeadUserCheck).GetSec() > mUserDeadSec/100)
      {
        mSaturn->ShootMIR( S_CleanUpDeadUsers() );
        mLastDeadUserCheck = now;
        stamp_p = true;
      }
      if ((now - mLastDeadServCheck).GetSec() > mServDeadSec/100)
      {
        mSaturn->ShootMIR( S_CleanUpDeadServers() );
        mLastDeadServCheck = now;
        stamp_p = true;
      }
      if ((now - mLastIdentServCheck).GetSec() > mServIdentSec)
      {
        mSaturn->ShootMIR( S_CleanUpNoIdentServers() );
        mLastIdentServCheck = now;
        stamp_p = true;
      }
      if (stamp_p)
      {
        Stamp(FID());
      }
    }

    GTime::SleepMiliSec(30000);
  }
}

//------------------------------------------------------------------------------

void XrdMonSucker::CleanUpOldUsers()
{
  static const Exc_t _eh("XrdMonSucker::CleanUpOldUsers ");
  assert_MIR_presence(_eh, ZGlass::MC_IsDetached);

  GTime cut_time = GTime::ApproximateTime() - GTime(mUserKeepSec, 0);

  list<XrdDomain*> domains;
  CopyListByGlass<XrdDomain>(domains);

  for (list<XrdDomain*>::iterator di = domains.begin(); di != domains.end(); ++di)
  {
    XrdDomain *d = *di;
    list<XrdServer*> servers;
    d->CopyListByGlass<XrdServer>(servers, false, true);

    Int_t n_wiped = 0;
    for (list<XrdServer*>::iterator si = servers.begin(); si != servers.end(); ++si)
    {
      XrdServer *s = *si;

      n_wiped += s->RemovePrevUsersOlderThan(cut_time);

      s->DecEyeRefCount();
    }
    if (n_wiped > 0)
    {
      ZLog::Helper log(*mLog, GTime::ApproximateTime(), ZLog::L_Message, _eh);
      log.Form("Removed %d previous users for domain '%s'.", n_wiped, d->GetName());
    }
  }
}

void XrdMonSucker::CleanUpDeadUsers()
{
  static const Exc_t _eh("XrdMonSucker::CleanUpDeadUsers ");

  assert_MIR_presence(_eh, ZGlass::MC_IsDetached);

  GTime now = GTime::ApproximateTime();

  ZLog::Helper log(*mLog, now, ZLog::L_Message, _eh);

  list<XrdDomain*> domains;
  CopyListByGlass<XrdDomain>(domains);

  for (list<XrdDomain*>::iterator di = domains.begin(); di != domains.end(); ++di)
  {
    XrdDomain *d = *di;
    list<XrdServer*> servers;
    d->CopyListByGlass<XrdServer>(servers, false, true);

    Int_t n_wiped = 0;
    for (list<XrdServer*>::iterator si = servers.begin(); si != servers.end(); ++si)
    {
      XrdServer *s = *si;
      list<XrdUser*> users;
      s->CopyListByGlass<XrdUser>(users);

      for (list<XrdUser*>::iterator ui = users.begin(); ui != users.end(); ++ui)
      {
        XrdUser *u = *ui;
        Int_t delta;
        {
          GLensReadHolder _lck(u);
          delta = (Int_t) (now - u->RefLastMsgTime()).GetSec();
        }
        if (delta > mUserDeadSec)
        {
          ++n_wiped;
          disconnect_user_and_close_open_files(u, s, now);
        }
      }

      s->DecEyeRefCount();
    }
    if (n_wiped > 0)
    {
      log.SetTime(GTime::ApproximateTime());
      log.Form("Removed %d dead users for domain '%s'.", n_wiped, d->GetName());
    }
  }
}

//------------------------------------------------------------------------------

void XrdMonSucker::CleanUpDeadServers()
{
  static const Exc_t _eh("XrdMonSucker::CleanUpDeadServers ");

  assert_MIR_presence(_eh, ZGlass::MC_IsDetached);

  GTime now = GTime::ApproximateTime();

  ZLog::Helper log(*mLog, now, ZLog::L_Message, _eh);

  list<XrdDomain*> domains;
  CopyListByGlass<XrdDomain>(domains);

  for (list<XrdDomain*>::iterator di = domains.begin(); di != domains.end(); ++di)
  {
    XrdDomain *domain = *di;
    list<XrdServer*> servers;
    domain->CopyListByGlass<XrdServer>(servers, false, true);

    for (list<XrdServer*>::iterator si = servers.begin(); si != servers.end(); ++si)
    {
      XrdServer *server = *si;
      Int_t delta;
      {
        GLensReadHolder _lck(server);
        delta = (Int_t) (now - server->RefLastMsgTime()).GetSec();
      }
      if (delta > mServDeadSec)
      {
        log.SetTime(GTime::ApproximateTime());
        log.Form("Removing unactive server '%s'.", server->GetName());

	disconnect_server(server, domain, now);
      }

      server->DecEyeRefCount();
    }
  }
}

void XrdMonSucker::CleanUpNoIdentServers()
{
  static const Exc_t _eh("XrdMonSucker::CleanUpNoIdentServers ");

  assert_MIR_presence(_eh, ZGlass::MC_IsDetached);

  GTime now = GTime::ApproximateTime();

  ZLog::Helper log(*mLog, now, ZLog::L_Message, _eh);

  list<XrdDomain*> domains;
  CopyListByGlass<XrdDomain>(domains);

  for (list<XrdDomain*>::iterator di = domains.begin(); di != domains.end(); ++di)
  {
    XrdDomain *domain = *di;
    list<XrdServer*> servers;
    domain->CopyListByGlass<XrdServer>(servers, false, true);

    for (list<XrdServer*>::iterator si = servers.begin(); si != servers.end(); ++si)
    {
      XrdServer *server = *si;
      Int_t ident_delta, delta;
      {
        GLensReadHolder _lck(server);

	ident_delta = server->GetAvgSrvIdDelta();
	if (ident_delta <= 0) goto done;

        delta = (Int_t) (now - server->RefLastMsgTime()).GetSec();
      }
      if (delta > mServIdentCnt * ident_delta)
      {
        log.SetTime(GTime::ApproximateTime());
        log.Form("Removing unactive server '%s'.", server->GetName());

	disconnect_server(server, domain, now);
      }
    done:
      server->DecEyeRefCount();
    }
  }
}

//==============================================================================

void XrdMonSucker::EmitTraceRERay()
{
  bTraceAllNull = mTraceDN.IsNull() && mTraceHost.IsNull() && mTraceDomain.IsNull();
  if ( ! bTraceAllNull)
  {
    mTraceDN_RE    .Reset(mTraceDN, "o");
    mTraceHost_RE  .Reset(mTraceHost, "o");
    mTraceDomain_RE.Reset(mTraceDomain, "o");
  }
}
