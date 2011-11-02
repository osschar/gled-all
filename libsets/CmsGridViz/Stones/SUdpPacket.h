// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_SUdpPacket_H
#define CmsGridViz_SUdpPacket_H

#include <Rtypes.h>

#include "Gled/GTime.h"

class SUdpPacket
{
public:
  GTime           mRecvTime;
  vector<UChar_t> mAddr;
  UShort_t        mPort;
  vector<UChar_t> mMessage;

  SUdpPacket();
  ~SUdpPacket();

#include "SUdpPacket.h7"
  ClassDefNV(SUdpPacket, 1);
}; // endclass SUdpPacket

#endif
