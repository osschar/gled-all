// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdMonSucker.h"
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
}

XrdMonSucker::~XrdMonSucker()
{}

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

  sockaddr_in addr;
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port        = htons(mSuckPort);

  if (bind(mSocket, (sockaddr*) &addr, sizeof(sockaddr_in)) == -1)
    throw _eh + "bind failed: " + strerror(errno);

  TPMERegexp username_re("(\\w+)\\.(\\d+):(\\d+)@([^\\.]+)(?:\\.(.+))?", "o");
  TPMERegexp hostname_re("([^\\.]+)\\.(.*)", "o");
  TPMERegexp authinfo_re("^&p=(.*)&n=(.*)&h=(.*)&o=(.*)&r=(.*)&m=(.*)$", "o");

  std::vector<char> buffer(16384 + 16);
  ssize_t           buf_size = buffer.size();
  char             *buf      = &buffer[0];
  int               flags    = 0;

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
    Char_t   pseq = xmh->pseq;
    UShort_t plen = ntohs(xmh->plen);
    Int_t    stod = ntohl(xmh->stod);
    UInt_t   in4a = addr.sin_addr.s_addr; // Kept in net order
    UShort_t port = ntohs(addr.sin_port);

    xrdsrv_id  xsid(in4a, stod, port);
    xrd_hash_i xshi = m_xrd_servers.find(xsid);

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


      XrdServer *server = new XrdServer(GForm("%s %s : %hu : %d", hostname_re[2].Data(), hostname_re[1].Data(), port, stod), "",
					hostname_re[1], hostname_re[2], GTime(stod));

      // ZQueen::CheckIn() does write lock.
      mQueen->CheckIn(server);
      Add(server);

      xshi = m_xrd_servers.insert(make_pair(xsid, server)).first;
    }
    XrdServer *server = xshi->second;

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
      msg.Form("Message from %s.%s:%hu, c=%c, seq=%3hhu, len=%hu\n",
               server->GetHost(), server->GetDomain(), port, xmh->code, pseq, plen);

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
	msg += TString::Format("  DN=%s, VO=%s, Role=%s\n",
			       authinfo_re[6].Data(), authinfo_re[4].Data(), authinfo_re[5].Data());

	// ZQueen::CheckIn() does write lock.
	XrdUser *user = new XrdUser(uname, "",
                                    authinfo_re[6], authinfo_re[4], authinfo_re[5],
                                    host, domain, recv_time);
	mQueen->CheckIn(user);
	{
	  GLensReadHolder _lck(server);
	  server->AddUser(user, dict_id);
	}
	{
	  GLensReadHolder _lck(user);
	  user->SetServer(server);

          if ( ! bTraceAllNull && mTraceDN_RE.Match(authinfo_re[6]) &&
               mTraceHost_RE.Match(host) && mTraceDomain_RE.Match(domain))
          {
            user->SetTraceMon(true);
          }          
	}

	// XXXX Missing grep / create CmsXrdUser ... somewhere
      }
      else if (code == 'd')
      {
	msg += TString::Format("  path map -- id=%d, uname=%s path=%s\n", dict_id, prim, sec);
	XrdUser *user = server->FindUser(prim);
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
	  XrdFile *file = new XrdFile(sec);
	  mQueen->CheckIn(file);

	  // should lock all those ....
	  user->AddFile(file);
	  user->SetLastMsgTime(recv_time);
	  file->SetUser(user);
	  server->AddFile(file, dict_id);
	}
	else
	{
          TString uname(prim);
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
          fi = lc.update(ntohl(xmt.arg2.dictid));
          if (fi)
          {
            // Int_t rwoff = ntohl(arg.arg0.val);
            Int_t rwlen = ntohl(xmt.arg1.buflen);
            GLensReadHolder _lck(fi);
            if (rwlen >= 0)
            {
              fi->DeltaReadMB(rwlen / 1048576.0);
            }
            else
            {
              fi->DeltaWriteMB(-rwlen / 1048576.0);
            }
            fi->SetLastMsgTime(lc.fTime);
          }
        }
	else if (tt == XROOTD_MON_OPEN || tt == XROOTD_MON_CLOSE)
	{
	  fi = lc.update(ntohl(xmt.arg2.dictid));
          if (vrb) printf("  %2d: %s, file=%s\n", ti, ttn, fi ? fi->GetName() : "<nullo>");
          if (fi)
          {
            GLensReadHolder _lck(fi);
            fi->SetLastMsgTime(lc.fTime);
            if (tt == XROOTD_MON_OPEN)
            {
              fi->SetOpenTime(lc.fTime);
            }
            else
            {
              ULong64_t x;
              x = ntohl(xmt.arg0.rTot[1]);
              x <<= xmt.arg0.id[1];
              fi->SetRTotalMB(x / 1048576.0);
              x = ntohl(xmt.arg1.wTot);
              x <<= xmt.arg0.id[1];
              fi->SetWTotalMB(x / 1048576.0);
              fi->SetCloseTime(lc.fTime);
            }
          }
	}
	else if (tt == XROOTD_MON_DISC)
	{
	  assert(us == server->FindUser(ntohl(xmt.arg2.dictid)));
	  if (vrb) printf("  %2d: %s, user=%s\n", ti, ttn, us ? us->GetName() : "<nott-therrea>");
	  if (us)
	  {
	    GLensReadHolder _lck(us);
	    us->SetDisconnectTime(lc.fTime);
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

  if (mSuckerThread)
    throw _eh + "already running.";


  mSuckerThread = new GThread("XrdMonSucker-Sucker",
			      (GThread_foo) tl_Suck, this,
			      false);
  mSuckerThread->SetNice(20);
  mSuckerThread->Spawn();
}

void XrdMonSucker::StopSucker()
{
  static const Exc_t _eh("XrdMonSucker::StopSucker ");

  if (mSuckerThread == 0)
    throw _eh + "not running.";

  mSuckerThread->Cancel();
  mSuckerThread->Join();
  mSuckerThread = 0;
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
