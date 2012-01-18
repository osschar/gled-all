// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "UdpPacketTcpClient.h"
#include "UdpPacketTcpClient.c7"

#include "Stones/SMessage.h"
#include "Stones/SUdpPacket.h"
#include "Gled/GThread.h"

#include <TSocket.h>


// UdpPacketTcpClient

//______________________________________________________________________________
//
// TCP client for UdpPacketProcessor. Can deliver packets to several local
// consumers using GQueueSet.

ClassImp(UdpPacketTcpClient);

//==============================================================================

void UdpPacketTcpClient::_init()
{
  mHost = "localhost";
  mPort = 9940;
  mSocket = 0;
  mListenerThread = 0;
}

UdpPacketTcpClient::UdpPacketTcpClient(const Text_t* n, const Text_t* t) :
  ZGlass(n, t)
{
  _init();
}

UdpPacketTcpClient::~UdpPacketTcpClient()
{}

//==============================================================================

void* UdpPacketTcpClient::tl_ListenLoop(UdpPacketTcpClient* c)
{
  c->ListenLoop();
  return 0;
}

void UdpPacketTcpClient::ListenLoop()
{
  GSelector sel;
  sel.fRead.Add(mSocket);

  while (true)
  {
    GThread::CancelOn();
    sel.Select();
    GThread::CancelOff();

    for (GFdSet_i i = sel.fReadOut.begin(); i != sel.fReadOut.end(); ++i)
    {
      TSocket* s = (TSocket*) i->first;
      assert(s == mSocket);

      UInt_t len; mSocket->RecvRaw(&len, sizeof(UInt_t));
      len = net2host(len);
      UInt_t len2 = len + sizeof(UInt_t);

      UChar_t *buf = new UChar_t[len2];

      mSocket->RecvRaw(buf + sizeof(UInt_t), len);

      SMessage msg(buf, len2);

      printf("Got len=%u, typ=%u\n", len, msg.What());

      SUdpPacket *p = new SUdpPacket;

      p->NetStreamer(msg);

      printf("  Msg from %hhu.%hhu.%hhu.%hhu:%hu, len=%hu, N_consumers=%d\n",
             p->mAddr[0], p->mAddr[1], p->mAddr[2], p->mAddr[3],
             p->mPort, p->mBuffLen, mConsumerSet.GetSetSize());

      mConsumerSet.DeliverToQueues(p);
    }
  }
}

void UdpPacketTcpClient::ConnectAndListenLoop()
{
  static const Exc_t _eh("UdpPacketTcpClient::ConnectAndListenLoop ");

  mSocket = new TSocket(mHost, mPort);

  mListenerThread = new GThread("UdpPacketTcpClient-ListenLoop",
                                (GThread_foo) tl_ListenLoop, this,
                                false);
  mListenerThread->SetNice(0);
  mListenerThread->Spawn();
}

//==============================================================================

void UdpPacketTcpClient::RegisterConsumer(Queue_t* q)
{
  mConsumerSet.RegisterQueue(q);
}

void UdpPacketTcpClient::UnregisterConsumer(Queue_t* q)
{
  mConsumerSet.UnregisterQueue(q);
}