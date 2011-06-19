// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdMonSucker.h"
#include "XrdMonSucker.c7"

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

  TPMERegexp username_re("(\\w+).(\\d+):(\\d+)@(.*)", "o");

  std::vector<char> buffer(8192 + 16);
  ssize_t           buf_size = buffer.size();
  char             *buf      = &buffer[0];
  int               flags    = 0;

  Char_t   hostname[1024], servicename[1024];
  UShort_t port;

  while (true)
  {
    socklen_t slen = sizeof(sockaddr_in);

    ssize_t len = recvfrom(mSocket, buf, buf_size, flags,
			   (sockaddr*) &addr, &slen);
    if (len == -1)
    {
      ISwarn(_eh + "recvfrom failed: " + strerror(errno));
      continue;
    }

    if (len == 0)
    {
      ISwarn(_eh + "recvfrom returned 0, not expected.");
      continue;
    }

    getnameinfo((sockaddr*) &addr, slen, hostname, 1024, servicename, 1024,
		NI_NOFQDN | NI_DGRAM | NI_NUMERICHOST | NI_NUMERICSERV);

    XrdXrootdMonHeader *xmh = (XrdXrootdMonHeader*) buf;
    xmh->plen = ntohs(xmh->plen);
    xmh->stod = ntohl(xmh->stod);

    if (len != xmh->plen)
    {
      ISerr(_eh + GForm("message size mismatch: got %zd, xrd-len=%hu (bufsize=%zd).", len, xmh->plen, buf_size));
      // This means either our buf-size is too small or the other guy is pushing it.
      // Should probably stop reporting errors from this IP.
      // XXXX Does it really help having it on stack?
      // XXXX Anyway, do additional checks in here about buf-size, got less, etc.
      continue;
    }

    //xrdsrv_id sid;
    //sid .
    port = ntohs(addr.sin_port);

    //UInt_t crc = gcrc.Start(addr.sin_addr.s_addr)
    //               .Process(port)
    //               .Finish(xmh->stod);

    if (xmh->code != 't')
    {
      // Assuming IP4 address
      char *foo = (char*) &addr.sin_addr.s_addr;
      printf("Gotta len %-4ld, from %hhu.%hhu.%hhu.%hhu:%hu, c=%c, seq=%3hhu, len=%hu\n",
	     len, foo[0], foo[1], foo[2], foo[3], port,
	     xmh->code, xmh->pseq, xmh->plen);
      printf("  '%s' '%s'\n", hostname, servicename);
    }

    if (xmh->code != 't')
    {
      buf[xmh->plen] = 0;

      XrdXrootdMonMap *xmm = (XrdXrootdMonMap*) xmh;
      xmm->dictid = ntohl(xmm->dictid);

      if (xmh->code == 'u')
      {
	printf("  user map -- id=%u, uname=%s\n", xmm->dictid, xmm->info);
	TString uname(xmm->info);
	if (username_re.Match(uname) == 5)
	{
	  printf("  %s - %d - %d - %s\n", username_re[1].Data(), username_re[2].Atoi(), username_re[3].Atoi(), username_re[4].Data());
	}
      }
      else
      {
	char *sec = strstr(xmm->info, "\n");
	*(sec++) = 0;
	printf(" other %c -- id=%u, uname=%s other=%s\n", xmh->code, xmm->dictid, xmm->info, sec);
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
