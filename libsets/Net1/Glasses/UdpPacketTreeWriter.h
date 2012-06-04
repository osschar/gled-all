// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Net1_UdpPacketTreeWriter_H
#define Net1_UdpPacketTreeWriter_H

#include "Glasses/ZGlass.h"
#include "Gled/GQueue.h"

class ZLog;
class UdpPacketSource;
class SUdpPacket;

class TFile;
class TTree;
class TBranch;


class UdpPacketTreeWriter : public ZGlass
{
  MAC_RNR_FRIENDS(UdpPacketTreeWriter);

private:
  void _init();

protected:
  ZLink<ZLog>             mLog;        // X{GS} L{}
  ZLink<UdpPacketSource>  mSource;     // X{GS} L{}

  GQueue<SUdpPacket>      mUdpQueue;   //!

  GThread                *mWLThread;   //!

  TString                 mFilePrefix; // X{GS} Textor()  
  TFile                  *mFile;       //!
  TTree                  *mTree;       //!
  TBranch                *mBranch;     //!

  // static void* tl_WriteLoop(UdpPacketTreeWriter* w);
  // void WriteLoop();

  void open_file_create_tree();
  void write_tree_close_file();

public:
  UdpPacketTreeWriter(const Text_t* n="UdpPacketTreeWriter", const Text_t* t=0);
  virtual ~UdpPacketTreeWriter();

  void Start(); // X{Ed} 7 MButt()
  void Stop();  // X{Ed} 7 MButt()

#include "UdpPacketTreeWriter.h7"
  ClassDef(UdpPacketTreeWriter, 1);
}; // endclass UdpPacketTreeWriter

#endif
