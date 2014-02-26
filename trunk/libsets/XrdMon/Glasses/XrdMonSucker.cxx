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
#include "Stones/SNetResolver.h"
#include "Gled/GThread.h"

#include "XrdMon/XrdXrootdMonData.h"

#include <cerrno>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


namespace
{
  const Double_t One_MB = 1024 * 1024;
}

// XrdMonSucker

//______________________________________________________________________________
//
// Processes monitoring messages from a multitude of xrootd servers from
// various domains.
//
// Forward all io trace records (one by one, unpacked into function arguments
// already) to XrdFile that implements proper processing and optional
// storage. Flag bStoreIoInfo is passed to XrdFile on creation time and
// controls whether each individual request will be stored in a SXrdInfo
// structer for later detailed analysis. Statistical information about reads,
// vector reads and writes is always collected.


ClassImp(XrdMonSucker);

//==============================================================================

void XrdMonSucker::_init()
{
  bStoreIoInfo = false;

  mUserKeepSec   = 300;
  mUserDeadSec   = 86400;
  mServDeadSec   = 86400;
  mServIdentSec  = 300;
  mServIdentCnt  = 5;
  mCheckInterval = 30;

  mPacketCount = mSeqIdFailCount = 0;

  mSuckerThread = 0;
  mLastOldUserCheck  = mLastDeadUserCheck  =
  mLastDeadServCheck = mLastIdentServCheck = GTime(GTime::I_Never);
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
  // Called when a file is closed.
  //
  // Notification is sent to registered file-close-reporters (but only if all
  // file, user and server are non-null -- this can happen if information
  // sources keep sending information after the close (for a file) or
  // disconnect (for a user)).
  //
  // After that the file is removed from the list of open files via a MIR.

  if (file && user && server)
  {
    Stepper<XrdFileCloseReporter> stepper(*mFCReporters);
    while (stepper.step())
    {
      stepper->FileClosed(file, user, server);
    }
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
    Bool_t freshly_closed = false;
    {
      GLensReadHolder _lck(file);
      if (file->IsOpen())
      {
        file->RegisterFileClose(time);
        freshly_closed = true;
      }
    }
    if (freshly_closed)
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
  mSaturn->DelayedShootMIR( mQueen->S_RemoveLenses(server->GetPrevUsers()),
                            GTime::ApproximateFuture(mUserKeepSec + mCheckInterval + 10) );
  mSaturn->DelayedShootMIR( domain->S_RemoveAll(server),
                            GTime::ApproximateFuture(mUserKeepSec + mCheckInterval + 20) );
}

//==============================================================================

void XrdMonSucker::Suck()
{
  static const Exc_t _eh("XrdMonSucker::Suck ");

  TPMERegexp username_re("([^.]+)\\.(\\d+):(\\d+)@(.+)", "o");
  TPMERegexp authinfo_re("^&p=(.*)&n=(.*)&h=(.*)&o=(.*)&r=(.*)&g=(.*)&m=(.*)$", "o");
  TPMERegexp authxxxx_re("^&p=(.*)&n=(.*)&h=(.*)&o=(.*)&r=(.*)$", "o");

  TPMERegexp redir_re   ("(.*?):(.*)", "o");

  TPMERegexp srvinfo_re ("^&pgm=(.*)&ver=(.*)&inst=(.*)&port=(.*)&site=(.*)$", "o");

  SNetResolver resolver;

  while (true)
  {
    GQueuePopper<SUdpPacket> p(mUdpQueue);

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
    UShort_t plen = net2host(xmh->plen);
    Int_t    stod = net2host(xmh->stod);
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
      TString fqhn, host_name, domain_name;

      if ( ! resolver.resolve_fqhn(*p, fqhn))
      {
        log.Put(ZLog::L_Error, "Unable to figure out name or address of a new server.");
	continue;
      }
      if ( ! resolver.split_fqhn_to_host_domain_no_lookup(fqhn, host_name, domain_name))
      {
        log.Form(ZLog::L_Error, "Can not split fqhn '%s' to host domain parts.", fqhn.Data());
        continue;
      }

      log.Form(ZLog::L_Message, "New server: %s.%s:%hu stod=%d",
	       host_name.Data(), domain_name.Data(), port, stod);

      server = new XrdServer(GForm("%s.%s : %d : %hu", host_name.Data(), domain_name.Data(), stod, port),
                             "", host_name, domain_name, GTime(stod));
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
    TString error_str = server->CheckSequenceId(code, pseq);
    if ( ! error_str.IsNull())
    {
      log.Put(ZLog::L_Warning, error_str);
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

    // Check length of message .vs. length claimed by xrd.
    if (p->mBuffLen != plen)
    {
      log.Form(ZLog::L_Warning, "Message size mismatch: got %zd, xrd-len=%hu.", p->mBuffLen, plen);
      // This means either our buf-size is too small or the other guy is pushing it.
      // Should probably stop reporting errors from this IP.
      // XXXX Do additional checks in here about buf-size, got less, etc.
      continue;
    }

    if (code != 't' && code != 'f' && code != 'r')
    {
      TString msg;
      msg.Form("Message from %s.%s:%hu, c=%c, seq=%hhu, len=%hu",
               server->GetHost(), server->GetDomain(), port,
               xmh->code, pseq, plen);

      XrdXrootdMonMap *xmm     = (XrdXrootdMonMap*) p->mBuff;
      UInt_t           dict_id = net2host(xmm->dictid);

      (p->mBuff)[plen] = 0; // 0-terminate the buffer at packet length.

      char *prim = xmm->info;
      char *sec  = strstr(prim, "\n");
      if (sec) {
	*(sec++) = 0;
      }

      if (code == 'u')
      {
	msg += TString::Format("\n\tUser map -- id=%u, uname=%s\n\t\tsec=%s", dict_id, prim, sec ? sec : "<null>");

	TString uname(prim);

        if (username_re.Match(uname) != 5)
        {
          msg += " ... parse error matching uname.";
          log.Put(ZLog::L_Error, msg);
          continue;
        }

        TString     host, domain, serv_uname;
        TString     auth_host, auth_group, auth_dn, auth_vo, auth_role;
        TPMERegexp *auth_re_ptr  = 0;

        if (sec)
        {
          if (authinfo_re.Match(sec) == 8)
          {
            auth_group  =  authinfo_re[6];
            auth_dn     =  authinfo_re[7];
            auth_re_ptr = &authinfo_re;
          }
          else if (authxxxx_re.Match(sec) == 6)
          {
            auth_group  =  "<unknown>";
            auth_dn     =  "<unknown>";
            auth_re_ptr = &authxxxx_re;
          }

          if (auth_re_ptr)
          {
            TPMERegexp &re = *auth_re_ptr;

            auth_host = re[3];
            auth_vo   = re[4];
            auth_role = re[5];

            // DPM actuallty reports DN in &n field
            if (re[2].BeginsWith("/") && re[1] == "gsi") {
              auth_dn    = re[2];
            } else {
              serv_uname = re[2];
            }
          }

        }


        TString fqhn = ( ! auth_host.IsNull()              &&
                         ! resolver.is_numeric (auth_host) &&
                         ! resolver.is_nodomain(auth_host)    ) ?
          auth_host      :
          username_re[4];

        if ( ! resolver.split_fqhn_to_host_domain_with_lookup(fqhn, host, domain))
        {
          msg += " ... error figuring out client hostname.";
          log.Put(ZLog::L_Error, msg);
          continue;
        }

        if (resolver.f_local_domain)
        {
          domain = server->RefDomain();
        }

        Bool_t numeric_host = resolver.f_numeric;

        if (username_re[1] == mNagiosUser && host.BeginsWith(mNagiosHost) && domain.BeginsWith(mNagiosDomain))
        {
          // msg += TString::Format(" ... it is nagios, skipping it.\n");
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

        XrdUser *user = 0;
        try
        {
          if (auth_re_ptr)
          {
            msg += GForm("\n\tDN=%s, Host=%s, VO=%s, Role=%s, Group=%s",
                         auth_dn.Data(), auth_host.Data(), auth_vo.Data(), auth_role.Data(), auth_group.Data());

            user = new XrdUser(uname, "", auth_dn, auth_vo, auth_role, auth_group,
                               serv_uname, host, domain, numeric_host, recv_time);
          }
          else
          {
            msg += GForm("\n\tUnparsable auth-info: '%s'", sec);

            user = new XrdUser(uname, "", "", "", "", "",
                               "", host, domain, numeric_host, recv_time);
          }
          // ZQueen::CheckIn() does write lock.
          mQueen->CheckIn(user);

          {
            GLensWriteHolder _lck(server);
            server->AddUser(user, dict_id);
          }
          {
            GLensWriteHolder _lck(user);
            user->SetServer(server);
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
	      file->RegisterFileMapping(recv_time, bStoreIoInfo);
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
	msg += TString::Format("\n\tInfo map -- id=%u, uname=%s info=%s",
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
          continue;
        }
      }
      else if (code == '=')
      {
        srvinfo_re.Match(sec);
        TString site = srvinfo_re[5];

	GLensWriteHolder _lck(server);
	server->UpdateSrvIdTime(recv_time, site);

	msg += TString::Format("\n\tServerId -- uname=%s other=%s", prim, sec);
      }
      else
      {
	// Message types that are not processed:
	//   p - purge (FRM only)
	//   x - file transfer (FRM only)

	msg += TString::Format("\n\tOther %c -- id=%u, uname=%s other=%s", code, dict_id, prim, sec);
      }

      log.Put(msg);
    } // if not 't' or 'f' or 'r'

    else if (code == 't') // this is a trace message
    {
      struct local_cache
      {
	XrdServer        *fSrv;
	XrdXrootdMonBuff *fXmb;

	XrdFile          *fFile;
	UInt_t            fDictId;

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
          return net2host(trace(fN-1).arg1.Window) - net2host(trace(0).arg2.Window);
        }

	Bool_t next()
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
	      fTime = GTime(net2host(trace().arg2.Window));
	      fTimeStep = (n_div >= 1) ?
		(GTime(net2host(trace(fTiWEnd).arg1.Window)) - fTime).ToDouble() / n_div :
		0;

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

	XrdFile* update(UInt_t newid)
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
	    case XROOTD_MON_READV:  c = "rdv"; break;
	    case XROOTD_MON_READU:  c = "rdu"; break;
	    default:                c = "rw "; break;
	  }
	  return c;
	}

      };

      local_cache lc(server, (XrdXrootdMonBuff*) p->mBuff, plen);

      TString msg;

      std::map<XrdUser*, GTime> user_map;
      std::map<XrdFile*, GTime> file_map;

      while (lc.next())
      {
	XrdXrootdMonTrace &xmt = lc.trace();
	UChar_t            tt  = lc.trace_type();

        XrdFile *file = 0;
        XrdUser *user = 0;

        if (tt <= 0x7F)
        {
	  UInt_t dict_id = net2host(xmt.arg2.dictid);
          file = lc.update(dict_id);
          if (file)
          {
            Long64_t rwoff = net2host(xmt.arg0.val);
            Int_t    rwlen = net2host(xmt.arg1.buflen);

            GLensReadHolder _lck(file);
	    file->RegisterReadOrWrite(rwoff, rwlen, lc.fTime);
          }
        }
        else if (tt == XROOTD_MON_READV || tt == XROOTD_MON_READU)
        {
	  UInt_t dict_id = net2host(xmt.arg2.dictid);
          file = lc.update(dict_id);
          if (file)
          {
            UShort_t nels = net2host(xmt.arg0.sVal[1]);
            Int_t    rlen = net2host(xmt.arg1.buflen);
            UChar_t  vseq = xmt.arg0.id[1];

            GLensReadHolder _lck(file);
	    if (tt == XROOTD_MON_READV)
	    {
	      file->RegisterReadV(nels, rlen, lc.fTime, vseq);
	    }
	    else
	    {
	      file->RegisterReadU(nels, rlen, lc.fTime, vseq);
	    }
          }
        }
	else if (tt == XROOTD_MON_OPEN || tt == XROOTD_MON_CLOSE)
	{
	  UInt_t dict_id = net2host(xmt.arg2.dictid);
	  file = lc.update(dict_id);
          if (file)
          {
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
            }
            else
            {
	      {
		GLensReadHolder _lck(file);
		ULong64_t x;
		x = net2host(xmt.arg0.rTot[1]);
		x <<= xmt.arg0.id[1];
		file->SetRTotalMB(x / One_MB);
		x = net2host(xmt.arg1.wTot);
		x <<= xmt.arg0.id[2];
		file->SetWTotalMB(x / One_MB);

		file->RegisterFileClose(lc.fTime);
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
              user = file->GetUser();
              on_file_close(file, user, server);
            }
          }
	}
	else if (tt == XROOTD_MON_DISC)
	{
          UInt_t dict_id = net2host(xmt.arg2.dictid);
	  user = server->FindUser(dict_id);

          bool disconn_p = true;
          TString extra;
          if (xmt.arg0.id[1] & XROOTD_MON_FORCED) {
            extra += "(forced)";
          }
          if (xmt.arg0.id[1] & XROOTD_MON_BOUNDP) {
            disconn_p = false;
            extra += "(bound-path)";
          }

          msg += GForm("\n\tDisconnect%s user='%s'", extra.Data(), user ? user->GetName() : "<nil>");

	  if (disconn_p && user)
	  {
            disconnect_user_and_close_open_files(user, server, lc.fTime);
	  }
	}

        if (user == 0 && file != 0) user = file->GetUser();
        if (user)  user_map[user] = lc.fTime;
        if (file)  file_map[file] = lc.fTime;

      } // while trace entries

      for (std::map<XrdUser*, GTime>::iterator i = user_map.begin(); i != user_map.end(); ++i)
      {
        GLensReadHolder _lck(i->first);
        i->first->SetLastMsgTime(i->second);
      }
      for (std::map<XrdFile*, GTime>::iterator i = file_map.begin(); i != file_map.end(); ++i)
      {
        GLensReadHolder _lck(i->first);
        i->first->SetLastMsgTime(i->second);
      }

      if ( ! msg.IsNull())
      {
        TString txt;
        txt.Form("Trace from %s.%s:%hu, N=%d, dt=%d, seq=%hhu, len=%hu.",
                 server->GetHost(), server->GetDomain(), port,
                 lc.fN, lc.full_delta_time(), pseq, plen);
        txt += msg;
        log.Put(txt);
      }
    } // else if -- 't' trace message handling

    else if (code == 'f')
    {
      XrdXrootdMonFileHdr *fb = (XrdXrootdMonFileHdr*)(p->mBuff + sizeof(XrdXrootdMonHeader));

      if (fb->recType != XrdXrootdMonFileHdr::isTime)
      {
	log.Form(ZLog::L_Error, "%s fstream message does not begin with a XrdXrootdMonFileTOD record.",
		 server->GetName());
	continue;
      }

      Int_t n_to_read = net2host(fb->nRecs[1]);
      Double_t t0, t1;
      {
	XrdXrootdMonFileTOD *th = (XrdXrootdMonFileTOD*) fb;
	t0 = net2host(th->tBeg);
	t1 = net2host(th->tEnd);
      }
      Double_t dt = n_to_read > 1 ? (t1 - t0) / (n_to_read - 1) : 0;

      std::map<XrdUser*, GTime> user_map;
      std::map<XrdFile*, GTime> file_map;

      Int_t bytes_left = plen;

      for (Int_t i = 0; i < n_to_read; ++i)
      {
	Int_t rec_size = net2host(fb->recSize);
	if (rec_size <= 0 || rec_size > bytes_left)
	{
	  log.Form(ZLog::L_Error, "%s fstream record %d has length %d (obviously wrong).",
		   server->GetName(), i, rec_size);
	  break;
	}

	fb = (XrdXrootdMonFileHdr*) ((char*) fb + rec_size);
	bytes_left -= rec_size;

	// static const char* type_names[] = { "cls", "opn", "tim", "xfr", "dsc" };
	Int_t  typ = fb->recType;
	UInt_t fid = net2host(fb->fileID);  // strictly, should use userID union entry for uid for record type "disconnect"
	GTime  time(t0 + i * dt);

	XrdFile *file = 0;
	XrdUser *user = 0;

	if (typ == XrdXrootdMonFileHdr::isOpen)
	{
	  XrdXrootdMonFileOPN &opn = * (XrdXrootdMonFileOPN*) fb;
	  // flag 'hasRW' not checked.

	  if (fb->recFlag & XrdXrootdMonFileHdr::hasLFN)
	  {
	    UInt_t uid  = net2host(opn.ufn.user);
	    user = server->FindUser(uid);
	    if (user)
	    {
	      TString path(opn.ufn.lfn);

	      // create XrdFile
	      try
	      {
		file = new XrdFile(path);
		mQueen->CheckIn(file);
		{
		  GLensWriteHolder _lck(user);
		  user->AddFile(file);
		}
		{
		  GLensWriteHolder _lck(file);
		  file->SetUser(user);
		  file->RegisterFileMapping(time, false);
		  file->SetSizeMB(net2host(opn.fsz) / One_MB);
		}
		{
		  GLensWriteHolder _lck(server);
		  server->AddFile(file, fid);
		}

		on_file_open(file);
	      }
	      catch (Exc_t exc)
	      {
		log.Form(ZLog::L_Error, "Exception in XrdFile instantiation for fstream: %s", exc.Data());
	      }
	    }
	    else
	    {
	      log.Form(ZLog::L_Error, "fstream-open unknown user ... ignoring. This will be supported in the future.");
	    }
	  }
	  else
	  {
	    // Currently not supported ... need XrdServer->UnknownUser list, or sth.
	    // Or ... a generic, non-removable "unknown user".
	    // Hmmh, how hard will I have to fight for this guy not to be wiped
	    // by automatic cleaners?
	    log.Form(ZLog::L_Error, "fstream UID, LFN extension not provided ... this is required for now.");
	  }
	}
	else if (typ == XrdXrootdMonFileHdr::isXfr)
	{
	  XrdXrootdMonFileXFR &xfr = * (XrdXrootdMonFileXFR*) fb;
	  file = server->FindFile(fid);
	  if (file != 0)
	  {
	    {
	      GLensReadHolder _lck(file);
	      file->RegisterFStreamXfr(xfr.Xfr, time);
	    }
	    user = file->GetUser();
	  }
	  else
	  {
	    // Unknown file id
	    log.Form(ZLog::L_Warning, "%s.%s fstream-xfr unknown file-id %u ... ignoring.",  server->GetHost(), server->GetDomain(), fid);
	  }
	}
	else if (typ == XrdXrootdMonFileHdr::isClose)
	{
	  XrdXrootdMonFileCLS &cls = * (XrdXrootdMonFileCLS*) fb;

	  file = server->FindFile(fid);
	  if (file != 0)
	  {
	    {
	      GLensReadHolder _lck(file);
	      file->RegisterFStreamClose(cls, time);
	    }
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
	    user = file->GetUser();
	    on_file_close(file, user, server);
	  }
	  else
	  {
	    log.Form(ZLog::L_Warning, "fstream-close unknown file-id ... ignoring.");
	  }
	}
	else if (typ == XrdXrootdMonFileHdr::isDisc)
	{
	  user = server->FindUser(fid);
	  if (user)
	  {
	    disconnect_user_and_close_open_files(user, server, time);
	  }
	  else
	  {
	    log.Form(ZLog::L_Warning, "fstream-disconnect unknown user-id ... ignoring.");
	  }
	}
	else
	{
	  log.Form(ZLog::L_Error, "Unknown / unexpected message type %d in fstream message.", typ);
	}

	if (user)  user_map[user] = time;
	if (file)  file_map[file] = time;
      } // for fstream entries

      for (std::map<XrdUser*, GTime>::iterator i = user_map.begin(); i != user_map.end(); ++i)
      {
	GLensReadHolder _lck(i->first);
	i->first->SetLastMsgTime(i->second);
      }
      for (std::map<XrdFile*, GTime>::iterator i = file_map.begin(); i != file_map.end(); ++i)
      {
	GLensReadHolder _lck(i->first);
	i->first->SetLastMsgTime(i->second);
      }

    } // else if -- 'f' fstream message handling

    else if (code == 'r')
    {
      XrdXrootdMonBurr *rb = (XrdXrootdMonBurr*) p->mBuff;

      TString txt;
      txt.Form("Redirect trace from %s.%s:%hu, seq=%hhu, len=%hu.",
               server->GetHost(), server->GetDomain(), port, pseq, plen);
      log.Put(txt);

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
                                   net2host(rr->arg0.Window), net2host(rr->arg1.Window));
          }
          else
          {
            UInt_t   uid  = net2host(rr->arg1.dictid);
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
    } // else if -- 'r' redirect message handling

  } // while (true) main loop
}

//==============================================================================

void* XrdMonSucker::tl_Suck(XrdMonSucker* s)
{
  static const Exc_t _eh("XrdMonSucker::tl_Suck ");

  GThread *thr = GThread::Self();
  s->mSaturn->register_detached_thread(s, thr);
  thr->CleanupPush((GThread_cu_foo) cu_Suck, s);

  {
    GLensReadHolder _lck(s);
    s->bSuckerRunning = true;
    s->Stamp(s->FID());
  }

  s->mSource->RegisterConsumer(&s->mUdpQueue);
  while (true)
  {
    try
    {
      s->Suck();
    }
    catch (Exc_t exc)
    {
      if (*(s->mLog))
        s->mLog->Put(ZLog::L_Error, _eh, exc + "\n\tTrying to reenter, this might not be the best idea.");
    }
  }

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

    GTime::SleepMiliSec(1000 * mCheckInterval);
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

      try
      {
        n_wiped += s->RemovePrevUsersOlderThan(cut_time);
      }
      catch (Exc_t exc)
      {
        ZLog::Helper log(*mLog, GTime::ApproximateTime(), ZLog::L_Error, _eh);
        log.Put(exc);
      }

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
