// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_UdpPacketProcessor_H
#define CmsGridViz_UdpPacketProcessor_H

#include "Glasses/ZList.h"

#include "Gled/GQueue.h"
#include "Gled/GSelector.h"
#include "Gled/GTime.h"

class ZLog;
class SUdpPacket;

class TServerSocket;
class TFile; class TTree; class TBranch;


class UdpPacketProcessor : public ZList
{
  MAC_RNR_FRIENDS(UdpPacketProcessor);

private:
  void _init();

protected:
  ZLink<ZLog>       mLog;          // X{GS} L{}

  Int_t             mSuckPort;     // X{GS} 7 Value()
  Int_t             mSocket;       //!
  GThread          *mSuckerThread; //!

  GQueue<SUdpPacket> mUdpQueue;

  static void* tl_Suck(UdpPacketProcessor* s);
  void Suck();

  //--------------------------------

  GSelector         mSelector;   //!
  GThread          *mServThread; //!
  TServerSocket    *mServSocket; //!
  Int_t             mServPort;   // X{GS} 7 Value()
  list<TSocket*>    mClients;

  static void* tl_Serve(UdpPacketProcessor* s);
  void Serve();

  void AddClient(TSocket* cs);
  void RemoveClient(TSocket* cs);
  void RemoveClient(list<TSocket*>::iterator sli);

  //--------------------------------

  GThread          *mDeliThread;   //!

  static void* tl_Deliver(UdpPacketProcessor* s);
  void Deliver();

  //--------------------------------

  TFile            *mDFile;   //!
  TTree            *mDTree;   //!
  TBranch          *mDBranch; //!

public:
  UdpPacketProcessor(const Text_t* n="UdpPacketProcessor", const Text_t* t=0);
  virtual ~UdpPacketProcessor();

  void StartAllServices(); // X{Ed} 7 MButt()
  void StopAllServices();  // X{Ed} 7 MButt()

#include "UdpPacketProcessor.h7"
  ClassDef(UdpPacketProcessor, 1);
}; // endclass UdpPacketProcessor

#endif
