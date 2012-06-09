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
class GSignal;

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

  Int_t                   mAutoSaveEntries;  // X{GS} 7 Value(-range=>[0, 1000000, 1])
  Int_t                   mRotateMinutes;    // X{GS} 7 Value(-range=>[0, 14400, 1])
  GTime                   mLastFileOpen;     //!

  GQueue<SUdpPacket>      mUdpQueue;   //!

  Bool_t                  bRunning;    //! X{G} 7 BoolOut()

  GThread                *mWLThread;   //!

  TString                 mFilePrefix; // X{GS} 7 Textor()  
  TFile                  *mFile;       //!
  TTree                  *mTree;       //!
  TBranch                *mBranch;     //!

  void open_file_create_tree();
  void write_tree_close_file();

  static void* tl_WriteLoop(UdpPacketTreeWriter* w);
  static void  cu_WriteLoop(UdpPacketTreeWriter* w);
  static void  sh_WriteLoop(GSignal* s);
  void WriteLoop();

public:
  UdpPacketTreeWriter(const Text_t* n="UdpPacketTreeWriter", const Text_t* t=0);
  virtual ~UdpPacketTreeWriter();

  void Start();      // X{Ed} 7 MButt()
  void Stop();       // X{Ed} 7 MButt()

  void RotateTree(); // X{E}  7 MButt()

#include "UdpPacketTreeWriter.h7"
  ClassDef(UdpPacketTreeWriter, 1);
}; // endclass UdpPacketTreeWriter

#endif
