// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_SUdpPacket_H
#define CmsGridViz_SUdpPacket_H

#include <Rtypes.h>

#include "Gled/GTime.h"
#include "Stones/SRefCounted.h"

class TBuffer;

class SUdpPacket : public SRefCountedNV

{
  // Not implemented
public:
  GTime           mRecvTime;
  Int_t           mBuffLen;
  UShort_t        mAddrLen;
  UShort_t        mPort;
  UChar_t         mAddr[16];
  UChar_t        *mBuff;      //[mBuffLen]

  SUdpPacket();
  SUdpPacket(const GTime& t, UChar_t* addr, UShort_t addr_len, UShort_t port,
             UChar_t* buff, Int_t buff_len);
  ~SUdpPacket();

  void  NetStreamer(TBuffer& b);
  Int_t NetBufferSize() const;

#include "SUdpPacket.h7"
  ClassDefNV(SUdpPacket, 1);
}; // endclass SUdpPacket

#endif
