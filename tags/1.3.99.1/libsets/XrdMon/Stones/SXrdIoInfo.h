// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef XrdMon_SXrdIoInfo_H
#define XrdMon_SXrdIoInfo_H

#include "Rtypes.h"
#include "TString.h"

#include <vector>

//==============================================================================

class SXrdReq
{
public:
  enum Req_e { R_Write, R_Read, R_VecRead };

private:
  Long64_t mOffset;   // Overloaded for R_VecRead
  Int_t    mLength;
  Int_t    mTime;

  void set_req_count(UShort_t rc)  { assign_offset(0xffff, 32, rc); }
  void set_reqs_lost(UShort_t rm)  { assign_offset(0x7fff, 48, rm); }

  void assign_offset(Long64_t mask, Int_t shift, Long64_t value)
  {
    value <<= shift;
    mask  <<= shift;
    mOffset &= ~mask;
    mOffset |=  value;
  }

public:
  SXrdReq() : mOffset(0), mLength(0), mTime(0) {}

  SXrdReq(Long64_t off, Int_t len, Int_t time) :
    mOffset(off), mLength(len), mTime(time) {}

  SXrdReq(Int_t index, UShort_t n_seg, Int_t len, Int_t time) :
    mOffset((1ll << 63) | (0xffffffffll & index) | (Long64_t(n_seg) << 32)),
    mLength(len), mTime(time) {}

  void IncLength(Int_t len)         { mLength += len; }
  void IncSubReqCount(UShort_t cnt) { set_req_count(SubReqCount() + cnt); }
  void IncSubReqsLost(UShort_t cnt) { set_reqs_lost(SubReqsLost() + cnt); }

  // Access functions

  Req_e Type() const
  {
    if (mOffset < 0) return R_VecRead;
    if (mLength < 0) return R_Write; else return R_Read;
  }
  Int_t  Length() const { return mLength < 0 ? -mLength : mLength; }
  Int_t  Time()   const { return mTime; }

  // For Read and Write
  Long64_t Offset() const { return mOffset < 0 ? -1 : mOffset; }

  // For VecRead
  Int_t    SubReqIndex() const   { return mOffset & 0xffffffff; }
  UShort_t SubReqCount() const   { return (mOffset >> 32) & 0xffff; }
  UShort_t SubReqsLost() const   { return (mOffset >> 48) & 0x7fff; }
  UShort_t SubReqsStored() const { return SubReqIndex() >= 0 ? SubReqCount() - SubReqsLost() : 0; }

  const char* TypeName() const
  {
    if (mOffset < 0) return "VecRead";
    if (mLength < 0) return "Write"; else return "Read";
  }

  ClassDefNV(SXrdReq, 1);
}; // endclass SXrdReq

typedef std::vector<SXrdReq> vSXrdReq_t;
typedef vSXrdReq_t::iterator vSXrdReq_i;


//==============================================================================

class SXrdIoInfo
{
public:
  vSXrdReq_t            mReqs;
  Int_t                 mNErrors;

  // Details for unpacked vector reads.
  std::vector<Long64_t> mOffsetVec;
  std::vector<Int_t>    mLengthVec;

  // ----------------------------------------------------------------

  SXrdIoInfo() : mNErrors(0) {}
  ~SXrdIoInfo() {}

  void Dump(Int_t level=1);

  ClassDefNV(SXrdIoInfo, 1);
}; // endclass SXrdIoInfo

#endif
