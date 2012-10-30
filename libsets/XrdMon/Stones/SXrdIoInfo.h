// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef XrdMon_SXrdIoInfo_H
#define XrdMon_SXrdIoInfo_H

#include "Rtypes.h"
#include <vector>

//==============================================================================

class SXrdReq
{
public:
  enum Req_e { R_Write, R_Read, R_VecRead };

private:
  Long64_t mOffset;
  Int_t    mLength;
  Int_t    mSeconds;

public:
  SXrdReq() : mOffset(0), mLength(0), mSeconds(0) {}

  void SetWrite()   {}
  void SetRead()    {}
  void SetVecRead() {}
  void AddVecRead() {}

  Req_e Type() const
  {
    if (mOffset < 0) return R_VecRead;
    if (mLength < 0) return R_Write; else return R_Read;
  }
  Int_t  Length()  const { return mLength < 0 ? -mLength : mLength; }
  Int_t  Seconds() const { return mSeconds; }

  // For Read and Write
  Long64_t Offset() const { return mOffset; }

  // For VecRead
  Int_t    SubReqIndex() const { return mOffset & 0xffffffff; }
  Short_t  SubReqLen()   const { return (mOffset > 32) & 0xffff; }
  Short_t  SubReqsLost() const { return (mOffset > 48) & 0xffff; }

    ClassDefNV(SXrdReq, 1);
}; // endclass SXrdReq

//==============================================================================

class SXrdRwReq
{
public:
  Long64_t          mOffset;
  Int_t             mLength;
  Int_t             mTime;

  SXrdRwReq()  {}
  ~SXrdRwReq() {}

  ClassDefNV(SXrdRwReq, 1);
}; // endclass SXrdRwReq

typedef std::vector<SXrdRwReq> vSXrdRwReq_t;

//==============================================================================

class SXrdRvSubReq
{
public:
  Long64_t          mOffset;
  Int_t             mLength;

  SXrdRvSubReq()  {}
  ~SXrdRvSubReq() {}

  ClassDefNV(SXrdRvSubReq, 1);
}; // endclass SXrdRvSubReq

typedef std::vector<SXrdRvSubReq> vSXrdRvSubReq_t;

//==============================================================================

class SXrdRvReq
{
public:
  Int_t             mNSubReqs;
  Int_t             mTotalLength;
  Int_t             mTime;

  vSXrdRvSubReq_t   mSubReqs;

  SXrdRvReq()  {}
  ~SXrdRvReq() {}

  ClassDefNV(SXrdRvReq, 1);
}; // endclass SXrdRvReq

typedef std::vector<SXrdRvReq> vSXrdRvReq_t;

//==============================================================================

class SXrdIoInfo
{
public:
  // Request info
  std::vector<Int_t>    mTimeVec;
  std::vector<Int_t>    mIndexVec; // Index into IO info
  std::vector<Short_t>  mTypeVec;  // -2 write, -1 read, >= 0 N_segments of vector read

  // IO info, there are several entries for vector reads.
  std::vector<Long64_t> mOffsetVec; // First entry for readv is number of segments reported in initial record
  std::vector<Int_t>    mLengthVec;

  UChar_t   mLastVSeq;          //! Should be in XrdFile?
  Short_t   mExpectedReadVSegs; //! Hmmh ... how about this guy?

  // Previous plan that couldn't replay time order
  vSXrdRwReq_t      mWrites;
  vSXrdRwReq_t      mReads;
  vSXrdRvReq_t      mReadvs;

  SXrdIoInfo()  {}
  ~SXrdIoInfo() {}

  void RegisterRead(Int_t time, Long64_t offset, Int_t length);
  void RegisterWrite(Int_t time, Long64_t offset, Int_t length);

  void RegisterReadV(Int_t time, Int_t n_segments, Int_t total_length);
  void ExtendReadV  (Int_t n_segments, Int_t total_length); // Happens for multi-file ReadV
  void RegisterReadVSeg(Long64_t offset, Int_t length);

  ClassDefNV(SXrdIoInfo, 1);
}; // endclass SXrdIoInfo

#endif
