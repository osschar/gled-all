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

public:
  SXrdReq() : mOffset(0), mLength(0), mTime(0) {}
  SXrdReq(Long64_t o, Int_t l, Int_t t) : mOffset(o), mLength(l), mTime(t) {}

  // Filling functions

  void SetWrite()   {}
  void SetRead()    {}
  void SetVecRead() {}
  void AddVecRead() {}

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
  Int_t    SubReqIndex() const { return mOffset & 0xffffffff; }
  Short_t  SubReqCount() const { return (mOffset > 32) & 0xffff; }
  Short_t  SubReqsLost() const { return (mOffset > 48) & 0x7fff; }

    ClassDefNV(SXrdReq, 1);
}; // endclass SXrdReq

typedef std::vector<SXrdReq> vSXrdReq_t;


//==============================================================================

class SXrdIoInfo
{
public:
  vSXrdReq_t            mReqs;

  // Details for unpacked vector reads.
  std::vector<Long64_t> mOffsetVec;
  std::vector<Int_t>    mLengthVec;

  // ----------------------------------------------------------------

  SXrdIoInfo()  {}
  ~SXrdIoInfo() {}


  ClassDefNV(SXrdIoInfo, 1);
}; // endclass SXrdIoInfo

#endif
