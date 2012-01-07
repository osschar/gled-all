// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SUdpPacket.h"

// SUdpPacket

//______________________________________________________________________________
//
//

ClassImp(SUdpPacket);

//==============================================================================

SUdpPacket::SUdpPacket() :
  mBuffLen(0), mAddrLen(0), mPort(0), mBuff(0)
{}

SUdpPacket::SUdpPacket(const GTime& t,
                       UChar_t* addr, UShort_t addr_len, UShort_t port,
                       UChar_t* buff, Int_t buff_len)
{
  mRecvTime = t;
  mBuffLen = buff_len;
  mAddrLen = addr_len;
  mPort = port;
  memcpy(mAddr, addr, addr_len);
  mBuff = new UChar_t[buff_len];
  memcpy(mBuff, buff, buff_len);
}

SUdpPacket::~SUdpPacket()
{
  delete [] mBuff;
}

//==============================================================================

void SUdpPacket::NetStreamer(TBuffer& b)
{
  if (b.IsReading())
  {
    UShort_t old_buff_len = mBuffLen;
    mRecvTime.NetStreamer(b);
    b >> mPort >> mAddrLen >> mBuffLen;
    b.ReadFastArray(mAddr, mAddrLen);
    if (mBuffLen != old_buff_len)
    {
      delete [] mBuff;
      mBuff = new UChar_t[mBuffLen];
    }
    b.ReadFastArray(mBuff, mBuffLen);
  }
  else
  {
    mRecvTime.NetStreamer(b);
    b << mPort << mAddrLen << mBuffLen;
    b.WriteFastArray(mAddr, mAddrLen);
    b.WriteFastArray(mBuff, mBuffLen);
  }
}

Int_t SUdpPacket::NetBufferSize() const
{
  return mRecvTime.NetBufferSize() + sizeof(Int_t) + 2 * sizeof(UShort_t) +
         mAddrLen + mBuffLen;
}
