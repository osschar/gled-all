// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "UdpPacketProcessor.h"
#include "UdpPacketProcessor.c7"
#include "UdpPacketTcpClient.h"

#include "Stones/SUdpPacket.h"
#include "Stones/SMessage.h"

#include "CmsGridViz/XrdXrootdMonData.h"

#include "Gled/GThread.h"

#include <TServerSocket.h>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>

#include <cerrno>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// UdpPacketProcessor

//______________________________________________________________________________
//
// Listens for UDP packets from xrootd servers (or anything, really).
// Stamps them with arrival time, sender ip4 + port and size.
// Writes it out into a root tree.
// Forwards it to a set of clients, over TCP.
//
// Message type 444:
//   - GTime
//   - 4-byte IP4 address
//   - 2-byte port
//   - 2-byte original message length
//   - the original message
//
// The service runs in three threads:
// 1. Sucker -- listents for UDP packets, creates SUdpPacket objects and passes
//    them on to Deli thread.
// 2. Deliverer (Deli) -- process the SUdpPacket objects: writes them to a TTree
//    and forwards them to connected clients.
// 3. Server (Serv) -- listens for incoming TCP network connections and handles
//    incoming traffic from those sockets.
//
// Local delivery is not implemented ... the plan is to have a list of local
// clients and send them the pointer directly. This would also require
// ref-counting on SUdpPackets.

ClassImp(UdpPacketProcessor);

//==============================================================================

void UdpPacketProcessor::_init()
{
  mSuckPort     = 9930;
  mSocket       = 0;
  mSuckerThread = 0;

  mServThread = 0;
  mServSocket = 0;
  mServPort   = 9940;

  mDeliThread = 0;

  mDFile   = 0;
  mDTree   = 0;
  mDBranch = 0;
}

UdpPacketProcessor::UdpPacketProcessor(const Text_t* n, const Text_t* t) :
  ZList(n, t),
  mSelector(GMutex::recursive)
{
  _init();
  SetElementFID(UdpPacketTcpClient::FID());
}

UdpPacketProcessor::~UdpPacketProcessor()
{}

//==============================================================================

void* UdpPacketProcessor::tl_Suck(UdpPacketProcessor* s)
{
  s->Suck();
  s->mSuckerThread = 0;
  return 0;
}

void UdpPacketProcessor::Suck()
{
  static const Exc_t _eh("UdpPacketProcessor::Suck ");

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

  std::vector<UChar_t> buffer(16384 + 16);
  ssize_t              buf_size = buffer.size();
  UChar_t             *buf      = &buffer[0];
  int                  flags    = 0;

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

    UInt_t   in4a = addr.sin_addr.s_addr; // Kept in net order
    UShort_t port = ntohs(addr.sin_port);

    // XrdXrootdMonHeader *xmh = (XrdXrootdMonHeader*) buf;
    // Char_t   code = xmh->code;
    // UChar_t  pseq = xmh->pseq;
    // UShort_t plen = ntohs(xmh->plen);
    // Int_t    stod = ntohl(xmh->stod);

    /*
    xrdsrv_id  xsid(in4a, stod, port);
    xrd_hash_i xshi = m_xrd_servers.find(xsid);
    */

    UChar_t *addr_p = (UChar_t*) &addr.sin_addr.s_addr;

    SUdpPacket *pp =  new SUdpPacket(recv_time, addr_p, 4, port, buf, len);

    // Send off to another thread here.

    /*
    mDBranch->SetAddress(&pp);

    mDTree->Fill();

    printf("Fill, n=%lld\n", mDTree->GetEntries());

    if (mDTree->GetEntries() % 1000 == 0)
    {
      mDTree->AutoSave("SaveSelf");
    }
    */

    {
      Char_t   hn_buf[64];
      getnameinfo((sockaddr*) &addr, slen, hn_buf, 64, 0, 0, NI_DGRAM);

      TString fqhn(hn_buf);
      fqhn.ToLower();

      char *foo = (char*) &in4a;
      printf("Message from: %hhu.%hhu.%hhu.%hhu:%hu, fqdn=%s\n",
             foo[0], foo[1], foo[2], foo[3], port, hn_buf);
    }

    mUdpQueue.PushBack(pp);

    // pseq fixing, too? Hmmh ... when to quit?

    /*
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
  }
}

//==============================================================================

void* UdpPacketProcessor::tl_Serve(UdpPacketProcessor* s)
{
  s->Serve();
  return 0;
}

void UdpPacketProcessor::Serve()
{
  static const Exc_t _eh("UdpPacketProcessor::Serve ");

  GThread::SetCancelType(GThread::CT_Deferred);

  mSelector.Clear();
  mSelector.fRead.Add(mServSocket);

  while (true)
  {
    GThread::CancelOn();
    mSelector.Select();
    GThread::CancelOff();

    for (GFdSet_i i=mSelector.fReadOut.begin(); i!=mSelector.fReadOut.end(); i++)
    {
      TSocket* s = (TSocket*) i->first;
      if (s == mServSocket)
      {
        TSocket *cs = mServSocket->Accept();

        Info(_eh, "Connection from %s:%d.",
             cs->GetInetAddress().GetHostName(), cs->GetLocalPort());

        AddClient(cs);
      }
      else
      {
        Info(_eh, "Message from some other socket, ignoring.");

        // Message from some other socket.
        // Can be close ... which is the only thing we MUST handle, to the first order.

        // see how to check for close.

        // write client, just dumping goit this, that ... and disconnect.
      }
    }

  }
}

void UdpPacketProcessor::AddClient(TSocket *cs)
{
  mSelector.Lock();
  mSelector.fRead.Add(cs);
  mClients.push_back(cs);
  mSelector.Unlock();
}

void UdpPacketProcessor::RemoveClient(TSocket* cs)
{
  mSelector.Lock();
  list<TSocket*>::iterator sli = find(mClients.begin(), mClients.end(), cs);
  if (sli != mClients.end())
  {
    RemoveClient(sli);
  }
  mSelector.Unlock();
}

void UdpPacketProcessor::RemoveClient(list<TSocket*>::iterator sli)
{
  TSocket *cs = *sli;
  mSelector.Lock();
  mSelector.fRead.Remove(*sli);
  mClients.erase(sli);
  mSelector.Unlock();
  delete cs;
}

//==============================================================================

void* UdpPacketProcessor::tl_Deliver(UdpPacketProcessor* s)
{
  s->Deliver();
  return 0;
}

void UdpPacketProcessor::Deliver()
{
  // wait on condifiton, while queue not empty, deliver to all sockets

  static const Exc_t _eh("UdpPacketProcessor::Deliver ");

  /*
  SUdpPacket *pup = 0;

  mDFile   = TFile::Open("xxx.root", "recreate");
  mDTree   = new TTree("Packets", "UDP packets");
  mDTree->SetAutoFlush(-300000);
  mDBranch = mDTree->Branch("P", &pup, 4096, 2);
  */

  while (true)
  {
    SUdpPacket *p = mUdpQueue.PopFront();

    cout << _eh << "Okdoki, spitting a message\n";

    // Write to root tree

    // Loop over clients
    SMessage msg(444, p->NetBufferSize());
    p->NetStreamer(msg);
    msg.SetLength();

    mSelector.Lock();
    list<TSocket*>::iterator i = mClients.begin();
    while (i != mClients.end())
    {
      Int_t len = (*i)->SendRaw(msg.Buffer(), msg.Length());

      if (len != msg.Length())
      {
        cout << _eh << "sent too little!\n";
      }

      if (len < 0)
      {
        cout << _eh << "other guy closed connection, it seems.\n";
        list<TSocket*>::iterator j = i++;
        RemoveClient(j);
      }
      else
      {
        ++i;
      }
    }
    mSelector.Unlock();

    delete p;
  }
}


//==============================================================================

void UdpPacketProcessor::StartAllServices()
{
  static const Exc_t _eh("UdpPacketProcessor::StartAllServices ");

  {
    GLensReadHolder _lck(this);
    if (mDeliThread || mSuckerThread || mServThread)
      throw _eh + "already running.";

    mServSocket = new TServerSocket(mServPort);
    if (!mServSocket->IsValid())
    {
      delete mServSocket; mServSocket = 0;
      throw _eh + "Creation of server socket failed.";
    }

    mDeliThread = new GThread("UdpPacketProcessor-Deliverer",
                              (GThread_foo) tl_Deliver, this,
                              false);
    mDeliThread->SetNice(10);

    mSuckerThread = new GThread("UdpPacketProcessor-Sucker",
                                (GThread_foo) tl_Suck, this,
                                false);
    mSuckerThread->SetNice(0);

    mServThread = new GThread("UdpPacketProcessor-Server",
                                (GThread_foo) tl_Serve, this,
                                false);
    mServThread->SetNice(20);
  }

  mDeliThread  ->Spawn();
  mSuckerThread->Spawn();
  mServThread  ->Spawn();
}

void UdpPacketProcessor::StopAllServices()
{
  static const Exc_t _eh("UdpPacketProcessor::StopAllServices ");

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

  mServThread->Kill();
  mServThread->Join();
  // ?? delete mServThread; later ?? just set to 0 ??
  {
    /*
    std::auto_ptr<TList> socks(fMonitor->GetListOfActives());
    while ( ! socks->IsEmpty())
    {
      TObject *obj = socks->First();
      socks->RemoveFirst();
      delete obj;
    }
    */
  }
  // delete fMonitor;
  // delete fMutex;


  printf("Writing tree ...\n");
  mDTree->Write();
  printf("Closing and deleting file ...\n");
  mDFile->Close();
  delete mDFile; mDFile = 0;

  {
    GLensReadHolder _lck(this);
    mSuckerThread = 0;
    mSocket = 0;
  }
}
