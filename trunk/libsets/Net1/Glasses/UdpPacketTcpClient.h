// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Net1_UdpPacketTcpClient_H
#define Net1_UdpPacketTcpClient_H

#include "Glasses/UdpPacketSource.h"

class GThread;
class SSocket;


class UdpPacketTcpClient : public UdpPacketSource
{
  MAC_RNR_FRIENDS(UdpPacketTcpClient);

private:
  void _init();

protected:
  TString           mHost; // X{GS} 7 Textor()
  UShort_t          mPort; // X{GS} 7 Value(-range=>[0,65535,1])

  SSocket          *mSocket;         //!
  GThread          *mListenerThread; //!

  static void* tl_ListenLoop(UdpPacketTcpClient* c);
  void ListenLoop();

public:
  UdpPacketTcpClient(const Text_t* n="UdpPacketTcpClient", const Text_t* t=0);
  virtual ~UdpPacketTcpClient();

  void ConnectAndListenLoop(); // X{Ed} 7 MButt()
  void StopListening(Bool_t close_p=true);        // X{Ed} 7 MCWButt()

  void SendMessage(); // X{Ed} 7 MButt()

#include "UdpPacketTcpClient.h7"
  ClassDef(UdpPacketTcpClient, 1);
}; // endclass UdpPacketTcpClient

#endif
