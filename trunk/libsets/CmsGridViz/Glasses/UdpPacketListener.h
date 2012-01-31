// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_UdpPacketListener_H
#define CmsGridViz_UdpPacketListener_H

#include <Glasses/UdpPacketSource.h>


class UdpPacketListener : public UdpPacketSource
{
  MAC_RNR_FRIENDS(UdpPacketListener);

private:
  void _init();

protected:
  Int_t             mSuckPort;     // X{GS} 7 Value()
  Int_t             mSocket;       //!
  GThread          *mSuckerThread; //!

  static void* tl_Suck(UdpPacketListener* s);
  void Suck();


public:
  UdpPacketListener(const Text_t* n="UdpPacketListener", const Text_t* t=0);
  virtual ~UdpPacketListener();

  void StartAllServices(); // X{Ed} 7 MButt()
  void StopAllServices();  // X{Ed} 7 MButt()

#include "UdpPacketListener.h7"
  ClassDef(UdpPacketListener, 1);
}; // endclass UdpPacketListener

#endif
