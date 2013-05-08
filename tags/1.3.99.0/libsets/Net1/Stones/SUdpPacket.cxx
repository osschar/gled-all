// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SUdpPacket.h"

// SUdpPacket

//______________________________________________________________________________
//
// A stone encapsulating a received UDP packet. It includes the address and
// port of the sender as well as the receive time.
//
// Note that the internal buffer is always one byte longer than what is
// reported in mBuffLen. This is usefull if one wants to inject a 0 or \n at
// the end of the buffer. The extra byte is never included in serialization!

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
  mBuff = new UChar_t[buff_len + 1]; // Extra byte, for \n, 0, etc.
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
      mBuff = new UChar_t[mBuffLen + 1]; // Extra byte, for \n, 0, etc.
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

UInt_t SUdpPacket::Ip4AsUInt() const
{
  const void *vp = mAddr;
  return * ((const UInt_t*) vp);
}
