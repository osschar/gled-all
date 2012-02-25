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
  UdpPacketSource(n, t)
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
  static const Exc_t _eh("UdpPacketTcpClient::ListenLoop ");

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

      SMessage *m = SMessage::ReceiveOrReport(s, _eh);
      if (m)
      {
        // printf("Recevied message, buffer_size=%d, type=%u\n", m->BufferSize(), m->What());

        SUdpPacket *p = new SUdpPacket;
        p->NetStreamer(*m);
        delete m;

        // printf("  Msg from %hhu.%hhu.%hhu.%hhu:%hu, len=%hu, N_consumers=%d\n",
        //        p->mAddr[0], p->mAddr[1], p->mAddr[2], p->mAddr[3],
        //        p->mPort, p->mBuffLen, mConsumerSet.GetSetSize());

        mConsumerSet.DeliverToQueues(p);
      }
      else
      {
        // if (s->TestBit(TSocket::kBrokenConn)) close etc ... schedule reconnect
      }
    }
  }
}

void UdpPacketTcpClient::ConnectAndListenLoop()
{
  static const Exc_t _eh("UdpPacketTcpClient::ConnectAndListenLoop ");

  {
    GLensReadHolder _lck(this);
    if (mListenerThread)
      throw _eh + "already running.";

    mSocket = new TSocket(mHost, mPort);

    mListenerThread = new GThread("UdpPacketTcpClient-ListenLoop",
                                  (GThread_foo) tl_ListenLoop, this,
                                  false);
    mListenerThread->SetNice(0);
  }

  mListenerThread->Spawn();
}

void UdpPacketTcpClient::StopListening(Bool_t close_p)
{
  static const Exc_t _eh("UdpPacketTcpClient::StopListening ");

  GThread *thr = 0;
  {
    GLensReadHolder _lck(this);
    if ( ! GThread::IsValidPtr(mListenerThread))
      throw _eh + "not running.";
    thr = mListenerThread;
    GThread::InvalidatePtr(mListenerThread);
  }

  thr->Cancel();
  thr->Join();
  if (close_p)
    mSocket->Close();
  delete mSocket;
  mSocket = 0;

  {
    GLensReadHolder _lck(this);
    mListenerThread = 0;
  }
}

//==============================================================================

void UdpPacketTcpClient::SendMessage()
{
  SMessage msg(666);
  msg << "Hello from HELL!";
  msg.Send(mSocket, true);
}
