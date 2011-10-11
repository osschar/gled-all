// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdMonSucker.h"
#include "XrdFileCloseReporter.h"
#include "Glasses/ZHashList.h"
#include "XrdMonSucker.c7"
#include "XrdServer.h"
#include "XrdUser.h"
#include "XrdFile.h"

#include "Gled/GThread.h"
#include "Gled/GCRC32.h"

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
//

ClassImp(XrdMonSucker);

//==============================================================================

void XrdMonSucker::_init()
{
  mSuckPort  = 9929;

  mNagiosUser   = "nagios";
  mNagiosHost   = "red-mon";
  mNagiosDomain = "unl.edu";

  mSocket = 0;
  mSuckerThread = 0;

  bTraceAllNull = true;
}

XrdMonSucker::XrdMonSucker(const Text_t* n, const Text_t* t) :
  ZNameMap(n, t)
{
  _init();
  SetElementFID(ZNameMap::FID());
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

//==============================================================================

void* XrdMonSucker::tl_Suck(XrdMonSucker* s)
{
  s->Suck();
  // delete s->mSock; s->mSock = 0;
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

    xrdsrv_id  xsid(in4a, stod, port);
    xrd_hash_i xshi = m_xrd_servers.find(xsid);

    XrdServer *server = 0;

    if (xshi == m_xrd_servers.end())
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


      server = new XrdServer(GForm("%s.%s : %hu : %d", hostname_re[1].Data(), hostname_re[2].Data(), port, stod), "",
                             hostname_re[1], hostname_re[2], GTime(stod));

      ZNameMap *domain = static_cast<ZNameMap*>(GetElementByName(server->GetDomain()));
      if (!domain)
      {
        domain = new ZNameMap(server->GetDomain());
        // ZQueen::CheckIn() does write lock.
        mQueen->CheckIn(domain);
        domain->SetKeepSorted(true);
        domain->SetElementFID(XrdServer::FID());
        Add(domain);
      }
      
      // ZQueen::CheckIn() does write lock.
      mQueen->CheckIn(server);
      domain->Add(server);

      xshi = m_xrd_servers.insert(make_pair(xsid, server)).first;

      server->InitSrvSeq(pseq);
    }
    else
    {
      server = xshi->second;

      UChar_t srv_seq = server->IncAndGetSrvSeq();
      if (pseq != srv_seq)
      {
        ISwarn(_eh + GForm("%s Sequence-id mismatch at '%s' srv=%hhu, loc=%hhu. Ignoring.",
                           recv_time.ToDateTimeLocal().Data(), server->GetName(),
                           srv_seq, pseq));
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

        authinfo_re.Match(sec);
	msg += TString::Format("  DN=%s, VO=%s, Role=%s, Group=%s\n",
			       authinfo_re[7].Data(), authinfo_re[4].Data(), authinfo_re[5].Data(), authinfo_re[6].Data());

	// ZQueen::CheckIn() does write lock.
	XrdUser *user = new XrdUser(uname, "", authinfo_re[7], authinfo_re[4], authinfo_re[5], authinfo_re[6],
                                    authinfo_re[2], host, domain, recv_time);
	mQueen->CheckIn(user);
	{
	  GLensReadHolder _lck(server);
	  server->AddUser(user, dict_id);
	}
	{
	  GLensReadHolder _lck(user);
	  user->SetServer(server);

          if ( ! bTraceAllNull && mTraceDN_RE.Match(authinfo_re[7]) &&
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

	  // should lock all those ....
	  user->AddFile(file);
	  user->SetLastMsgTime(recv_time);
	  file->SetUser(user);
	  server->AddFile(file, dict_id);

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
              fi->SetOpenTime(lc.fTime);
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
		server->RemoveFile(fi, dict_id);
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
	  if (vrb) printf("  %2d: %s, user=%s\n", ti, ttn, us ? us->GetName() : "<nil>");
	  if (us)
	  {
            {
              GLensReadHolder _lck(us);
              us->SetDisconnectTime(lc.fTime);
            }
            list<XrdFile*> open_files;
            us->CopyListByGlass<XrdFile>(open_files);
            for (list<XrdFile*>::iterator xfi = open_files.begin(); xfi != open_files.end(); ++xfi)
            {
              Bool_t closed = false;
              {
                GLensReadHolder _lck(*xfi);
                if ((*xfi)->IsOpen())
                {
                  (*xfi)->SetCloseTime(lc.fTime);
                  closed = true;
                }
              }
              if (closed)
              {
                on_file_close(*xfi);
              }
            }

            {
              GLensReadHolder _lck(server);
              server->DisconnectUser(us, dict_id);
            }
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

    mSuckerThread = new GThread("XrdMonSucker-Sucker",
                                (GThread_foo) tl_Suck, this,
                                false);
  }
  mSuckerThread->SetNice(0);
  mSuckerThread->Spawn();
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

  thr->Cancel();
  thr->Join();
  close(mSocket);
  {
    GLensReadHolder _lck(this);
    mSuckerThread = 0;
    mSocket = 0;
  }
}

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
