// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "UdpPacketTcpServer.h"
#include "UdpPacketSource.h"
#include "Glasses/ZLog.h"
#include "UdpPacketTcpServer.c7"

#include "Stones/SServerSocket.h"
#include "Stones/SMessage.h"
#include "Stones/SUdpPacket.h"
#include "Gled/GThread.h"

#include <cerrno>


// UdpPacketTcpServer

//______________________________________________________________________________
//
//

ClassImp(UdpPacketTcpServer);

//==============================================================================

void UdpPacketTcpServer::_init()
{
  mServThread = 0;
  mServSocket = 0;
  mServPort   = 9940;

  mDeliThread = 0;
}

UdpPacketTcpServer::UdpPacketTcpServer(const Text_t* n, const Text_t* t) :
  ZGlass(n, t),
  mSelector(GMutex::recursive)
{
  _init();
}

UdpPacketTcpServer::~UdpPacketTcpServer()
{}

//==============================================================================

void* UdpPacketTcpServer::tl_Serve(UdpPacketTcpServer* s)
{
  s->Serve();
  return 0;
}

void UdpPacketTcpServer::Serve()
{
  static const Exc_t _eh("UdpPacketTcpServer::Serve ");

  GThread::SetCancelType(GThread::CT_Deferred);

  mSelector.Clear();
  mSelector.fRead.Add(mServSocket);

  while (true)
  {
    GThread::CancelOn();
    mSelector.Select();
    GThread::CancelOff();

    for (GFdSet_i i = mSelector.fReadOut.begin(); i != mSelector.fReadOut.end(); ++i)
    {
      SSocket* s = (SSocket*) i->first;
      if (s == mServSocket)
      {
        // New client.

        SSocket *cs = mServSocket->Accept();
        if (*mLog)
        {
          mLog->Form(ZLog::L_Message, _eh, "New connection from %s.",
                     cs->GetInetAddress().GetHostName());
        }
        AddClient(cs);
      }
      else
      {
        // Message from some other socket.
        // Can be close ... which is the only thing we MUST handle, to the
        // first order.
        // Eventually support other stuff, like streaming/compression control,
        // play-back, etc. Maybe.

        SMessage *m = SMessage::ReceiveOrReport(s, _eh, true, *mLog);
        if (m)
        {
          if (*mLog)
          {
            mLog->Form(ZLog::L_Message, _eh, "Got messge from %s, len=%u, what=%u.",
                       s->GetInetAddress().GetHostName(), m->Length(), m->What());
          }
          delete m;
        }
        else if (s->TestBit(SSocket::kBrokenConn))
        {
          RemoveClient(s);
        }
      }
    }
  }
}

void UdpPacketTcpServer::AddClient(SSocket *cs)
{
  mSelector.Lock();
  mSelector.fRead.Add(cs);
  mClients.push_back(cs);
  mSelector.Unlock();
}

void UdpPacketTcpServer::RemoveClient(SSocket* cs)
{
  mSelector.Lock();
  list<SSocket*>::iterator sli = find(mClients.begin(), mClients.end(), cs);
  if (sli != mClients.end())
  {
    RemoveClient(sli);
  }
  mSelector.Unlock();
}

void UdpPacketTcpServer::RemoveClient(list<SSocket*>::iterator sli)
{
  SSocket *cs = *sli;
  mSelector.Lock();
  mSelector.fRead.Remove(*sli);
  mClients.erase(sli);
  mSelector.Unlock();
  delete cs;
}

//==============================================================================

void* UdpPacketTcpServer::tl_Deliver(UdpPacketTcpServer* s)
{
  s->Deliver();
  return 0;
}

void UdpPacketTcpServer::Deliver()
{
  // wait on condition, while queue not empty, deliver to all sockets

  static const Exc_t _eh("UdpPacketTcpServer::Deliver ");

  while (true)
  {
    SUdpPacket *p = mUdpQueue.PopFront();

    // Loop over clients
    SMessage msg(444, p->NetBufferSize());
    p->NetStreamer(msg);
    msg.SetLength();

    mSelector.Lock();
    list<SSocket*>::iterator i = mClients.begin();
    while (i != mClients.end())
    {
      try
      {
        msg.Send(*i, false);
      }
      catch (Int_t err)
      {
        if (*mLog)
        {
          mLog->Form(ZLog::L_Error, _eh,
                     "Error %d sending to %s:%d (%s). Closing connection.",
                     err, (*i)->GetInetAddress().GetHostName(), (*i)->GetLocalPort(),
                     strerror(errno));
        }
        list<SSocket*>::iterator j = i++;
        RemoveClient(j);
        continue;
      }
      ++i;
    }
    mSelector.Unlock();

    p->DecRefCount();
  }
}

//==============================================================================

void UdpPacketTcpServer::StartAllServices()
{
  static const Exc_t _eh("UdpPacketTcpServer::StartAllServices ");

  // XXX Assert some stuff about source

  {
    GLensReadHolder _lck(this);
    if (mDeliThread || mServThread)
      throw _eh + "already running.";

    mServSocket = new SServerSocket(mServPort, true);
    if (!mServSocket->IsValid())
    {
      delete mServSocket; mServSocket = 0;
      throw _eh + "Creation of server socket failed.";
    }

    mDeliThread = new GThread("UdpPacketTcpServer-Deliverer",
                              (GThread_foo) tl_Deliver, this,
                              false);
    mDeliThread->SetNice(10);

    mServThread = new GThread("UdpPacketTcpServer-Server",
                                (GThread_foo) tl_Serve, this,
                                false);
    mServThread->SetNice(20);
  }

  mSource->RegisterConsumer(&mUdpQueue);

  mDeliThread->Spawn();
  mServThread->Spawn();
}

void UdpPacketTcpServer::StopAllServices()
{
  static const Exc_t _eh("UdpPacketTcpServer::StopAllServices ");

  GThread *thr = 0;
  {
    GLensReadHolder _lck(this);
    if ( ! GThread::IsValidPtr(mServThread))
      throw _eh + "not running.";
    thr = mServThread;
    GThread::InvalidatePtr(mServThread);
  }

  // XXX Somewhere, close all sockets.

  mSource->UnregisterConsumer(&mUdpQueue);

  thr->Cancel();
  thr->Join();

  mDeliThread->Cancel();
  mDeliThread->Join();

  {
    GLensReadHolder _lck(this);
    mServThread = 0;
    mDeliThread = 0;
  }
}
