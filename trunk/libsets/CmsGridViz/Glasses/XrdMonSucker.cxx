// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdMonSucker.h"
#include "XrdFileCloseReporter.h"
#include "Glasses/ZHashList.h"
#include "XrdMonSucker.c7"
#include "XrdDomain.h"
#include "XrdServer.h"
#include "XrdUser.h"
#include "XrdFile.h"

#include "Gled/GThread.h"

#include "CmsGridViz/XrdXrootdMonData.h"

#include "TPRegexp.h"

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
  mServKeepSec = 86400;

  mSocket = 0;
  mSuckerThread = 0;
  mLastUserCheck = mLastServCheck = GTime(GTime::I_Never);

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
    mOpenFiles = new ZHashList("OpenFiles");
    mOpenFiles->SetElementFID(XrdFile::FID());
    mQueen->CheckIn(mOpenFiles.get());
  }
}


//==============================================================================

void XrdMonSucker::on_file_open(XrdFile* file)
{
  auto_ptr<ZMIR> mir( mOpenFiles->S_Add(file) );
  mSaturn->ShootMIR(mir);
}

void XrdMonSucker::on_file_close(XrdFile* file)
{
  auto_ptr<ZMIR> mir( mOpenFiles->S_Remove(file) );
  mSaturn->ShootMIR(mir);

  XrdFileCloseReporter *fcr = *mFCReporter;
  if (fcr)
  {
    fcr->FileClosed(file);
  }
}

//------------------------------------------------------------------------------

void XrdMonSucker::disconnect_user_and_close_open_files(XrdUser* user, XrdServer* server,
                                                        const GTime& time)
{
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
      if ((file)->IsOpen())
      {
        (file)->SetCloseTime(time);
        closed = true;
      }
    }
    if (closed)
    {
      on_file_close(file);
    }
  }

  {
    GLensReadHolder _lck(server);
    server->DisconnectUser(user);
  } 
}

//==============================================================================

void* XrdMonSucker::tl_Suck(XrdMonSucker* s)
{
  s->Suck();
  // Usually cancelled from StopSucker().
  s->mSuckerThread = 0;
  return 0;
}

void XrdMonSucker::Suck()
{
  static const Exc_t _eh("XrdMonSucker::Suck ");

  if ((mSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    throw _eh + "socket failed: " + strerror(errno);

  {
    struct addrinfo *result;
    struct addrinfo  hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags    = AI_PASSIVE | AI_NUMERICSERV;

    int error = getaddrinfo(0, TString::Format("%hu", mSuckPort), &hints, &result);
    if (error != 0)
      throw _eh + "getaddrinfo failed: " + gai_strerror(error);

    if (bind(mSocket, result->ai_addr, result->ai_addrlen) == -1)
      throw _eh + "bind failed: " + strerror(errno);

    freeaddrinfo(result);
  }

  TPMERegexp username_re("(\\w+)\\.(\\d+):(\\d+)@([^\\.]+)(?:\\.(.+))?", "o");
  TPMERegexp hostname_re("([^\\.]+)\\.(.*)", "o");
  TPMERegexp authinfo_re("^&p=(.*)&n=(.*)&h=(.*)&o=(.*)&r=(.*)&g=(.*)&m=(.*)$", "o");
  TPMERegexp authxxxx_re("^&p=(.*)&n=(.*)&h=(.*)&o=(.*)&r=(.*)$", "o");

  std::vector<char> buffer(16384 + 16);
  ssize_t           buf_size = buffer.size();
  char             *buf      = &buffer[0];
  int               flags    = 0;

  struct sockaddr_in addr;
  while (true)
  {
    socklen_t slen = sizeof(sockaddr_in);

    ssize_t len = recvfrom(mSocket, buf, buf_size - 1, flags,
			   (sockaddr*) &addr, &slen);
    if (len == -1)
    {
      ISwarn(_eh + "recvfrom failed: " + strerror(errno));
      continue;
    }
    else if (len == 0)
    {
      ISwarn(_eh + "recvfrom returned 0, not expected.");
      continue;
    }

    GTime recv_time(GTime::I_Now);

    XrdXrootdMonHeader *xmh = (XrdXrootdMonHeader*) buf;
    Char_t   code = xmh->code;
    UChar_t  pseq = xmh->pseq;
    UShort_t plen = ntohs(xmh->plen);
    Int_t    stod = ntohl(xmh->stod);
    UInt_t   in4a = addr.sin_addr.s_addr; // Kept in net order
    UShort_t port = ntohs(addr.sin_port);

    SXrdServerId xsid(in4a, stod, port);
    xrd_hash_i   xshi;
    bool         server_not_known;
    {
      GMutexHolder _lck(m_xrd_servers_mutex);
      xshi = m_xrd_servers.find(xsid);
      server_not_known = (xshi == m_xrd_servers.end());
    }

    XrdServer *server = 0;
    if (server_not_known)
    {
      Char_t   hn_buf[64];
      getnameinfo((sockaddr*) &addr, slen, hn_buf, 64, 0, 0, NI_DGRAM);

      TString fqhn(hn_buf);
      fqhn.ToLower();
      if (hostname_re.Match(fqhn) != 3)
      {
        char *foo = (char*) &in4a;
        printf("New server NS lookup problem: %hhu.%hhu.%hhu.%hhu:%hu, fqdn=%s\n",
               foo[0], foo[1], foo[2], foo[3], port, hn_buf);
	ISerr(_eh + GForm("Apparently a mis-formed FQ machine name: '%s'.", hn_buf));
	continue;
      }

      printf("New server: %s.%s:%hu'\n", hostname_re[1].Data(), hostname_re[2].Data(), port);


      server = new XrdServer(GForm("%s.%s : %d : %hu", hostname_re[1].Data(), hostname_re[2].Data(), stod, port),
                             "", hostname_re[1], hostname_re[2], GTime(stod));
      server->m_server_id = xsid;

      XrdDomain *domain = static_cast<XrdDomain*>(GetElementByName(server->GetDomain()));
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

      server->InitSrvSeq(pseq);
    }
    else
    {
      server = xshi->second;

      UChar_t srv_seq = server->IncAndGetSrvSeq();
      if (pseq != srv_seq)
      {
        ISwarn(_eh + GForm("%s Sequence-id mismatch at '%s' srv=%hhu, loc=%hhu; code=%c. Ignoring.",
                           recv_time.ToDateTimeLocal().Data(), server->GetName(),
                           srv_seq, pseq, code));
        server->InitSrvSeq(pseq);
      }
    }

    {
      GLensReadHolder _lck(server);
      server->SetLastMsgTime(recv_time);
    }

    if (len != plen)
    {
      ISerr(_eh + GForm("message size mismatch: got %zd, xrd-len=%hu (bufsize=%zd).", len, plen, buf_size));
      // This means either our buf-size is too small or the other guy is pushing it.
      // Should probably stop reporting errors from this IP.
      // XXXX Does it really help having it on stack?
      // XXXX Anyway, do additional checks in here about buf-size, got less, etc.
      continue;
    }

    if (code != 't')
    {
      TString msg;
      msg.Form("%s Message from %s.%s:%hu, c=%c, seq=%3hhu, len=%hu\n",
               recv_time.ToDateTimeLocal().Data(),
               server->GetHost(), server->GetDomain(), port,
               xmh->code, pseq, plen);

      XrdXrootdMonMap *xmm     = (XrdXrootdMonMap*) buf;
      Int_t            dict_id = ntohl(xmm->dictid);

      buf[plen] = 0; // 0-terminate the buffer at packet length.

      char *prim = xmm->info;
      char *sec  = strstr(prim, "\n");
      if (sec) {
	*(sec++) = 0;
      }

      if (code == 'u')
      {
	msg += TString::Format("  user map -- id=%d, uname=%s", dict_id, prim);
	TString uname(prim), host, domain;
        {
          Int_t nm = username_re.Match(uname);
          if (nm == 5)
          {
            // No domain, same as XrdServer
            msg += TString::Format(".%s\n", server->GetDomain());
            host   = username_re[4];
            domain = server->RefDomain();
          }
          else if (nm == 6)
          {
            // Domain given
            msg += TString::Format("\n");
            host   = username_re[4];
            domain = username_re[5];
          }
          else
          {
            msg += TString::Format(" ... parse error.\n");
            cout << msg;
            continue;
          }

          if (username_re[1] == mNagiosUser && host.BeginsWith(mNagiosHost) && domain.BeginsWith(mNagiosDomain))
          {
            // msg += TString::Format(" ... it is nagios, skipping it.\n");
            // cout << msg;
            continue;
          }

          if (server->ExistsUserDictId(dict_id))
          {
            msg += TString::Format("  XXXXXX Damn, dict_id already taken!\n");
            cout << msg;
            ISwarn(_eh + "user dict_id already taken ... this session will not be tracked.");
            continue;
          }
        }

        XrdUser *user = 0;
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
            msg += TString::Format("  unparsable auth-info: '%s'.\n", sec);
            cout << msg;
            // ISwarn(_eh + TString::Format("unparsable auth-info: '%s'.", sec));
          }

	  if (a_rep)
	  {
	    TPMERegexp &a_re = *a_rep;

	    msg += TString::Format("  DN=%s, VO=%s, Role=%s, Group=%s\n",
				   dn.Data(), a_re[4].Data(), a_re[5].Data(), group.Data());

	    user = new XrdUser(uname, "", dn, a_re[4], a_re[5], group,
			       a_re[2], host, domain, recv_time);
	  }
	  else
	  {
	    user = new XrdUser(uname, "", "", "", "", "",
			       "", host, domain, recv_time);
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

	// XXX Eventually ... grep / create CmsXrdUser.
      }
      else if (code == 'd')
      {
        TString uname(prim);
        TString path (sec);
	msg += TString::Format("  path map -- id=%d, uname=%s path=%s\n",
                               dict_id, uname.Data(), path.Data());
	XrdUser *user = server->FindUser(uname);
	if (user)
	{
	  if (server->ExistsFileDictId(dict_id))
	  {
	    msg += TString::Format("  XXXXXX Damn, dict_id already taken!\n");
            cout << msg;
	    ISwarn(_eh + "file dict_id already taken ... this file will not be tracked.");
	    continue;
	  }

	  // create XrdFile
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
	else
	{
          if ( ! uname.BeginsWith(mNagiosUser))
          {
            msg += TString::Format("  user not found ... skipping.\n");
            cout << msg;
          }
	  continue;
	}
      }
      else
      {
	msg += TString::Format("  other %c -- id=%u, uname=%s other=%s\n", code, dict_id, prim, sec);
      }

      cout << msg;
    }
    else // this is a trace message
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

	Bool_t next(Bool_t verbose=false)
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
                printf("  Window iB=%2d iE=%2d N=%2d delta_t=%f -- start = %s\n",
                       fTi, fTiWEnd, fTiWEnd-fTi-1, fTimeStep, fTime.ToDateTimeLocal().Data());

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
          for (Int_t i = 1; i < fN; ++i)
          {
            XrdXrootdMonTrace &xmt = trace(i);
            UChar_t tt = trace_type(i);
            if (tt <= 0x7F || tt == XROOTD_MON_OPEN || tt == XROOTD_MON_CLOSE)
            {
              update(ntohl(xmt.arg2.dictid));
              return fFile ? fFile->GetUser() : 0;
            }
            else if (tt == XROOTD_MON_DISC)
            {
              return fSrv->FindUser(ntohl(xmt.arg2.dictid));
            }
          }
          return 0;
        }
      };

      local_cache lc(server, (XrdXrootdMonBuff*) buf, plen);

      XrdUser *us = lc.find_user();
      Bool_t  vrb = us && us->GetTraceMon();

      if (vrb) printf("Trace from %s.%s:%hu, N=%d, dt=%d, seq=%hhu, len=%hu\n",
                      server->GetHost(), server->GetDomain(), port,
                      lc.fN, lc.full_delta_time(), pseq, plen);

      while (lc.next(vrb))
      {
	Int_t ti = lc.fTi;
	XrdXrootdMonTrace &xmt = lc.trace();
	UChar_t       tt  = lc.trace_type();
	const Char_t *ttn = lc.trace_type_name();

        // if (vrb) printf ("  %-2d: %hhx %s\n", ti, tt, ttn);

        XrdFile *fi = 0;

        if (tt <= 0x7F)
        {
	  Int_t dict_id = ntohl(xmt.arg2.dictid);
          fi = lc.update(dict_id);
          if (fi)
          {
            // Int_t rwoff = ntohl(arg.arg0.val);
            Int_t rwlen = ntohl(xmt.arg1.buflen);
            GLensReadHolder _lck(fi);
            if (rwlen >= 0)
            {
              fi->AddReadSample ( rwlen / One_MB);
            }
            else
            {
              fi->AddWriteSample(-rwlen / One_MB);
            }
            fi->SetLastMsgTime(lc.fTime);
          }
        }
        else if (tt == XROOTD_MON_READV)
        {
	  Int_t dict_id = ntohl(xmt.arg2.dictid);
          fi = lc.update(dict_id);
          if (fi)
          {
            Int_t rlen = ntohl(xmt.arg1.buflen);
            // Not processed: vcnt and vseq (for multi file read)
            GLensReadHolder _lck(fi);
            fi->AddReadSample (rlen / One_MB);
            fi->SetLastMsgTime(lc.fTime);
          }
        }
	else if (tt == XROOTD_MON_OPEN || tt == XROOTD_MON_CLOSE)
	{
	  Int_t dict_id = ntohl(xmt.arg2.dictid);
	  fi = lc.update(dict_id);
          if (vrb) printf("  %2d: %s, file=%s\n", ti, ttn, fi ? fi->GetName() : "<nullo>");
          if (fi)
          {
            if (tt == XROOTD_MON_OPEN)
            {
	      GLensReadHolder _lck(fi);
	      fi->SetLastMsgTime(lc.fTime);
            }
            else
            {
	      {
		GLensReadHolder _lck(fi);
		fi->SetLastMsgTime(lc.fTime);
		ULong64_t x;
		x = ntohl(xmt.arg0.rTot[1]);
		x <<= xmt.arg0.id[1];
		fi->SetRTotalMB(x / One_MB);
		x = ntohl(xmt.arg1.wTot);
		x <<= xmt.arg0.id[2];
		fi->SetWTotalMB(x / One_MB);
		fi->SetCloseTime(lc.fTime);
	      }
	      {
		GLensReadHolder _lck(server);
		server->RemoveFile(fi);
	      }
              on_file_close(fi);
            }
          }
	}
	else if (tt == XROOTD_MON_DISC)
	{
	  Int_t dict_id = ntohl(xmt.arg2.dictid);
	  XrdUser *us_from_server = server->FindUser(dict_id);
	  if (us != us_from_server)
	  {
	    ISwarn(_eh + GForm("us != us_from_server: us=%p ('%s'), us_from_server=%p ('%s')", us, us_from_server, us ? us->GetName() : "<nil>", us_from_server ? us_from_server->GetName() : "<nil>"));
	    printf("Jebojebo!\n");
            us = us_from_server;
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

	  if (vrb) printf("  %2d: %s%s, user=%s\n", ti, ttn, extra.Data(), us ? us->GetName() : "<nil>");
	  if (disconn_p && us)
	  {
            disconnect_user_and_close_open_files(us, server, lc.fTime);
	  }
          
	}
      } // while trace entries

      if (us)
      {
        GLensReadHolder _lck(us);
        us->SetLastMsgTime(lc.fTime);
      }

    } // else -- trace message handling

  } // while (true) main loop
}

//==============================================================================

void XrdMonSucker::StartSucker()
{
  static const Exc_t _eh("XrdMonSucker::StartSucker ");

  {
    GLensReadHolder _lck(this);
    if (mSuckerThread)
      throw _eh + "already running.";

    mSuckerThread  = new GThread("XrdMonSucker-Sucker",
                                 (GThread_foo) tl_Suck, this, false);

    mCheckerThread = new GThread("XrdMonSucker-Checker",
                                 (GThread_foo) tl_Check, this, false);
  }
  mSuckerThread->SetNice(0);
  mSuckerThread->Spawn();

  mLastUserCheck = mLastServCheck = GTime(GTime::I_Now);
  mCheckerThread->SetNice(20);
  mCheckerThread->Spawn();

  {
    GLensReadHolder _lck(this);
    Stamp(FID());
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

  mCheckerThread->Cancel();
  mCheckerThread->Join();
  delete mCheckerThread;

  thr->Cancel();
  thr->Join();
  delete thr;
  close(mSocket);
  {
    GLensReadHolder _lck(this);
    mSocket = 0;
    mSuckerThread = 0;
    mCheckerThread = 0;
  }
}

//==============================================================================

void* XrdMonSucker::tl_Check(XrdMonSucker* s)
{
  s->Check();
  // Usually cancelled from StopSucker().
  s->mCheckerThread = 0;
  return 0;
}

void XrdMonSucker::Check()
{
  static const Exc_t _eh("XrdMonSucker::Check ");

  while (true)
  {
    GTime now(GTime::I_Now);

    {
      bool stamp_p = false;
      GLensReadHolder _lck(this);
      if ((now - mLastUserCheck).GetSec() > mUserKeepSec)
      {
        mSaturn->ShootMIR( S_CleanUpOldUsers() );
        mLastUserCheck = now;
        stamp_p = true;
      }
      if ((now - mLastServCheck).GetSec() > mServKeepSec)
      {
        mSaturn->ShootMIR( S_CleanUpOldServers() );
        mLastServCheck = now;
        stamp_p = true;
      }
      if (stamp_p)
      {
        Stamp(FID());
      }
    }

    GTime::SleepMiliSec(10000);
  }
}

//------------------------------------------------------------------------------

void XrdMonSucker::CleanUpOldServers()
{
  static const Exc_t _eh("XrdMonSucker::CleanUpOldServers ");
  assert_MIR_presence(_eh, ZGlass::MC_IsDetached);

  GTime now(GTime::I_Now);

  list<XrdDomain*> domains;
  CopyListByGlass<XrdDomain>(domains);

  for (list<XrdDomain*>::iterator di = domains.begin(); di != domains.end(); ++di)
  {
    XrdDomain *domain = *di;
    list<XrdServer*> servers;
    domain->CopyListByGlass<XrdServer>(servers);

    for (list<XrdServer*>::iterator si = servers.begin(); si != servers.end(); ++si)
    {
      XrdServer *server = *si;
      Int_t delta;
      {
        GLensReadHolder _lck(server);
        delta = (Int_t) (now - server->RefLastMsgTime()).GetSec();
      }
      if (delta > mServKeepSec)
      {
        printf("%sRemoving unactive server '%s'.", _eh.Data(), server->GetName());
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
            disconnect_user_and_close_open_files(user, server, now);
          }
        }
        {
          // then remove the server from doman, it will get deleted
          //   when all the users are harvested
          GLensWriteHolder _lck(domain);
          domain->RemoveAll(server);
        }
        {
          mSaturn->ShootMIR( mQueen->S_RemoveLenses(server->GetPrevUsers()) );
          mSaturn->ShootMIR( mQueen->S_RemoveLens  (server) );
        }
      }
    }
  }
}

void XrdMonSucker::CleanUpOldUsers()
{
  static const Exc_t _eh("XrdMonSucker::CleanUpOldUsers ");
  assert_MIR_presence(_eh, ZGlass::MC_IsDetached);

  GTime now(GTime::I_Now);

  list<XrdDomain*> domains;
  CopyListByGlass<XrdDomain>(domains);

  for (list<XrdDomain*>::iterator di = domains.begin(); di != domains.end(); ++di)
  {
    XrdDomain *d = *di;
    list<XrdServer*> servers;
    d->CopyListByGlass<XrdServer>(servers);

    Int_t n_wiped = 0;
    for (list<XrdServer*>::iterator si = servers.begin(); si != servers.end(); ++si)
    {
      XrdServer *s = *si;
      list<XrdUser*> users;
      s->GetPrevUsers()->CopyListByGlass<XrdUser>(users);

      for (list<XrdUser*>::iterator ui = users.begin(); ui != users.end(); ++ui)
      {
        XrdUser *u = *ui;
        Int_t delta;
        {
          GLensReadHolder _lck(u);
          delta = (Int_t) (now - u->RefDisconnectTime()).GetSec();
        }
        if (delta > mUserKeepSec)
        {
          ++n_wiped;
          mQueen->RemoveLens(u);
        }
        else
        {
          // These are time-ordered ... so the rest are newer.
          break;
        }
      }
    }
    if (n_wiped > 0)
    {
      printf("%sRemoved %d previous users for domain '%s'.\n", _eh.Data(), n_wiped, d->GetName());
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
