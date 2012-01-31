// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_UdpPacketTreeWriter_H
#define CmsGridViz_UdpPacketTreeWriter_H

#include <Glasses/ZGlass.h>

class TFile; class TTree; class TBranch;

class UdpPacketTreeWriter : public ZGlass
{
  MAC_RNR_FRIENDS(UdpPacketTreeWriter);

private:
  void _init();

protected:
  TFile            *mDFile;   //!
  TTree            *mDTree;   //!
  TBranch          *mDBranch; //!

public:
  UdpPacketTreeWriter(const Text_t* n="UdpPacketTreeWriter", const Text_t* t=0);
  virtual ~UdpPacketTreeWriter();

#include "UdpPacketTreeWriter.h7"
  ClassDef(UdpPacketTreeWriter, 1);
}; // endclass UdpPacketTreeWriter

#endif
