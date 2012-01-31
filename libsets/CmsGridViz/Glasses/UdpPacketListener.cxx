// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "UdpPacketListener.h"
#include "UdpPacketListener.c7"

#include "Stones/SUdpPacket.h"

#include "Gled/GThread.h"

#include <cerrno>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// UdpPacketListener

//______________________________________________________________________________
//
//

ClassImp(UdpPacketListener);

//==============================================================================

void UdpPacketListener::_init()
{
  mSuckPort     = 9930;
  mSocket       = 0;
  mSuckerThread = 0;
}

UdpPacketListener::UdpPacketListener(const Text_t* n, const Text_t* t) :
  UdpPacketSource(n, t)
{
  _init();
}

UdpPacketListener::~UdpPacketListener()
{}

//==============================================================================

void* UdpPacketListener::tl_Suck(UdpPacketListener* s)
{
  s->Suck();
  s->mSuckerThread = 0;
  return 0;
}

void UdpPacketListener::Suck()
{
  static const Exc_t _eh("UdpPacketListener::Suck ");

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

  const int     buf_size = 65536;
  unsigned char buf[buf_size];
  int           flags = 0;

  struct sockaddr_in addr;
  socklen_t slen = sizeof(sockaddr_in);

  while (true)
  {
    ssize_t len = recvfrom(mSocket, buf, buf_size, flags,
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

    /*
      XrdXrootdMonHeader *xmh = (XrdXrootdMonHeader*) buf;
      Char_t   code = xmh->code;
      UChar_t  pseq = xmh->pseq;
      UShort_t plen = ntohs(xmh->plen);
      Int_t    stod = ntohl(xmh->stod);

      if (len != plen)
      {
      ISerr(_eh + GForm("message size mismatch: got %zd, xrd-len=%hu (bufsize=%zd).", len, plen, buf_size));
      // This means either our buf-size is too small or the other guy is pushing it.
      // Should probably stop reporting errors from this IP.
      // XXXX Does it really help having it on stack?
      // XXXX Anyway, do additional checks in here about buf-size, got less, etc.
      continue;
      }
    */

    UInt_t   in4a = addr.sin_addr.s_addr; // Kept in net order
    UShort_t port = ntohs(addr.sin_port);

    UChar_t *addr_p = (UChar_t*) &addr.sin_addr.s_addr;

    SUdpPacket *p =  new SUdpPacket(recv_time, addr_p, 4, port, buf, len);

    if (G_DEBUG > 1)
    {
      Char_t   hn_buf[64];
      getnameinfo((sockaddr*) &addr, slen, hn_buf, 64, 0, 0, NI_DGRAM);

      TString fqhn(hn_buf);
      fqhn.ToLower();

      char *foo = (char*) &in4a;
      printf("Message from: %hhu.%hhu.%hhu.%hhu:%hu, fqdn=%s, len=%zd\n",
             foo[0], foo[1], foo[2], foo[3], port, hn_buf, len);
    }

    mConsumerSet.DeliverToQueues(p);
  }
}

//==============================================================================

void UdpPacketListener::StartAllServices()
{
  static const Exc_t _eh("UdpPacketListener::StartAllServices ");

  {
    GLensReadHolder _lck(this);
    if (mSuckerThread)
      throw _eh + "already running.";

    mSuckerThread = new GThread("UdpPacketListener-Sucker",
                                (GThread_foo) tl_Suck, this,
                                false);
    mSuckerThread->SetNice(0);
  }

  mSuckerThread->Spawn();
}

void UdpPacketListener::StopAllServices()
{
  static const Exc_t _eh("UdpPacketListener::StopAllServices ");

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
