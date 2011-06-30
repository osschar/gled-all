// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdMonSucker.h"
#include "XrdMonSucker.c7"
#include "XrdServer.h"
#include "XrdUser.h"

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

  mSocket = 0;
  mSuckerThread = 0;
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

  std::vector<char> buffer(8192 + 16);
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
      char *foo = (char*) &in4a;
      printf("Yeehaa ... a server never seen before: %hhu.%hhu.%hhu.%hhu -- %hu\n",
	     foo[0], foo[1], foo[2], foo[3], port);
      
      Char_t   hn_buf[64];
      getnameinfo((sockaddr*) &addr, slen, hn_buf, 64, 0, 0, NI_DGRAM);

      TString fqhn(hn_buf);
      if (hostname_re.Match(fqhn) != 3)
      {
	ISerr(_eh + GForm("Apparently a mis-formed FQ machine name: '%s'.", hn_buf));
	continue;
      }

      printf("  I-yebo-ga zove se '%s' domain '%s'\n", hostname_re[1].Data(), hostname_re[2].Data());


      XrdServer *server = new XrdServer(GForm("%s %s : %hu - %d", hostname_re[2].Data(), hostname_re[1].Data(), port, stod), "",
					hostname_re[1], hostname_re[2], GTime(stod));

      // XXXX Lock & blabla or whatever etc.

      mQueen->CheckIn(server);
      Add(server);

      xshi = m_xrd_servers.insert(make_pair(xsid, server)).first;
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
      // Assuming IP4 address
      char *foo = (char*) &in4a;
      printf("Gotta len %-4ld, from %hhu.%hhu.%hhu.%hhu:%hu, c=%c, seq=%3hhu, len=%hu\n",
	     len, foo[0], foo[1], foo[2], foo[3], port, xmh->code, pseq, plen);
      printf("  '%s' \n", xshi->second->GetName());
    }

    if (code != 't')
    {
      XrdXrootdMonMap *xmm    = (XrdXrootdMonMap*) buf;
      Int_t            dictid = ntohl(xmm->dictid);

      buf[plen] = 0; // 0-terminate the buffer at packet length.

      char *prim = xmm->info;
      char *sec  = strstr(prim, "\n");
      if (sec) {
	*(sec++) = 0;
      }

      if (code == 'u')
      {
	printf("  user map -- id=%u, uname=%s\n", dictid, prim);
	TString uname(prim), host, domain;
	Int_t nm = username_re.Match(uname);
	if (nm == 5)
	{
	  // No domain, same as XrdServer
	  printf("  NOODOM %s - %d - %d - %s\n", username_re[1].Data(), username_re[2].Atoi(), username_re[3].Atoi(), username_re[4].Data());
	  host   = username_re[4];
	  domain = xshi->second->RefDomain();
	}
	else if (nm == 6)
	{
	  // Domain given
	  printf("  YESDOM %s - %d - %d - %s -- %s\n", username_re[1].Data(), username_re[2].Atoi(), username_re[3].Atoi(), username_re[4].Data(), username_re[5].Data());
 	  host   = username_re[4];
	  domain = username_re[5];
	}
	else
	{
	  printf("  WTFDOM JEBOJEBO no matcsha\n");
	  continue;
	}

	if (sec != 0)
	{
	  printf("  WWWWWW We have some actual user info, too -- '%s'.\n", sec);
	}

	// XXXX Lock & blabla or whatever etc.

	XrdUser *user = new XrdUser(uname, sec, host, domain, recv_time);
	mQueen->CheckIn(user);
	xshi->second->Add(user);

	user->SetServer(xshi->second);

	// XXXX Missing grep / create CmsXrdUser ... somewhere
     }
      else
      {
	
	// XXXXX must also zero-terminate the 'sec' guy !!!!
	printf(" other %c -- id=%u, uname=%s other=%s\n", code, dictid, prim, sec);
      }
    }
  }
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
