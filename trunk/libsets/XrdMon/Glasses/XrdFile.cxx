// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdFile.h"
#include "XrdUser.h"
#include "XrdFile.c7"

// XrdFile

//______________________________________________________________________________
//
// Various RegixterXyzz functions implement full, stateful processing of IO
// traces. XrdSucker invokes them as server monitoring messages come in.
//
// if bStoreIoInfo is set (flag usually set by XrdMonSucker on creation time)
// SXrdIoInfo stores individual IO requests in mIoInfo.

ClassImp(XrdFile);

//==============================================================================

void XrdFile::_init()
{
  m_dict_id = 0;

  mOpenTime.SetNever();
  mCloseTime.SetNever();
  mLastMsgTime.SetNever();

  mReadStats.Reset();
  mSingleReadStats.Reset();
  mVecReadStats.Reset();
  mVecReadCntStats.Reset();
  mWriteStats.Reset();
  mRTotalMB = mWTotalMB = 0;
  mSizeMB = 0;

  mExpectedReadVSegs = 0;
  mLastVSeq = 0xff;
  bStoreIoInfo = false;
}

XrdFile::XrdFile(const Text_t* n, const Text_t* t) :
  ZGlass(n, t)
{
  _init();
}

XrdFile::~XrdFile()
{}

//==============================================================================

void XrdFile::DumpIoInfo(Int_t level)
{
  mIoInfo.Dump(level);
}

//==============================================================================

void XrdFile::AddReadSample(Double_t x)
{
  mReadStats.AddSample(x);
  mSingleReadStats.AddSample(x);
}

void XrdFile::AddVecReadSample(Double_t x, Int_t n)
{
  mReadStats.AddSample(x);
  mVecReadStats.AddSample(x);
  mVecReadCntStats.AddSample(n);
}

void XrdFile::AddWriteSample(Double_t x)
{
  mWriteStats.AddSample(x);
}

//==============================================================================

namespace
{
  const Double_t One_MB = 1024 * 1024;
}

void XrdFile::RegisterFileMapping(const GTime& register_time, Bool_t store_io_info)
{
  bStoreIoInfo = store_io_info;
  mOpenTime    = register_time;
  mLastMsgTime = register_time;
  Stamp(FID());
}

void XrdFile::RegisterFileOpen(const GTime& open_time)
{
  mOpenTime    = open_time;
  mLastMsgTime = open_time;
  Stamp(FID());
}

void XrdFile::RegisterReadOrWrite(Long64_t offset, Int_t length, const GTime& time)
{
  if (length >= 0)
  {
    RegisterRead(offset, length, time);
  }
  else
  {
    RegisterWrite(offset, length, time);
  }
}

void XrdFile::RegisterRead(Long64_t offset, Int_t length, const GTime& time)
{
  // Process read segment record.
  // This can also be a sub-request of an unpacked vector-read event:
  // mExpectedReadVSegs is larger than zero in this case.

  // Note: exits function when true.
  if (mExpectedReadVSegs > 0)
  {
    if (bStoreIoInfo)
    {
      mIoInfo.mOffsetVec.push_back(offset);
      mIoInfo.mLengthVec.push_back(length);
    }
    --mExpectedReadVSegs;
    return;
  }

  if (bStoreIoInfo)
  {
    Int_t delta_t = (time - mOpenTime).GetSec();
    mIoInfo.mReqs.push_back(SXrdReq(offset, length, delta_t));
  }

  AddReadSample(length / One_MB);
  mLastMsgTime = time;
  Stamp(FID());
}

void XrdFile::RegisterWrite(Long64_t offset, Int_t length, const GTime& time)
{
  // Expects length to be negative (but this is also checked & enforced).

  end_read_vseg_if_expected();

  // Make sure length is negative.
  if (length > 0) length = -length;

  if (bStoreIoInfo)
  {
    Int_t delta_t = (time - mOpenTime).GetSec();
    mIoInfo.mReqs.push_back(SXrdReq(offset, length, delta_t));
  }
  AddReadSample(length / One_MB);
  mLastMsgTime = time;
  Stamp(FID());
}

//------------------------------------------------------------------------------

void XrdFile::RegisterReadV(UShort_t n_segments, Int_t total_length, const GTime& time, UChar_t vseq)
{
  // ReadV record, unpacked info is *not* following.
  // Additional records with the same vseq are possible.

  end_read_vseg_if_expected();

  if (bStoreIoInfo)
  {
    if (mLastVSeq != vseq)
    {
      Int_t delta_t = (time - mOpenTime).GetSec();
      mIoInfo.mReqs.push_back(SXrdReq(-1, n_segments, total_length, delta_t));
    }
    else
    {
      mIoInfo.mReqs.back().IncLength(total_length);
      mIoInfo.mReqs.back().IncSubReqCount(n_segments);
    }
  }

  mLastVSeq = vseq;

  // Ignore multi-file vector reads with several entries from the same file,
  // treat them as separate.
  // Would have to sum stuff up and commit it from end_read_vseg_if_expected()
  // but it seems I'd need one more state var (Bool_t bInReadV).
  AddVecReadSample(total_length / One_MB, n_segments);

  mLastMsgTime = time;
  Stamp(FID());
}

void XrdFile::RegisterReadU(UShort_t n_segments, Int_t total_length, const GTime& time, UChar_t vseq)
{
  // ReadV record, unpacked info is about to follow.
  // Additional records with the same vseq are possible but only after individual reads.

  end_read_vseg_if_expected();

  if (bStoreIoInfo)
  {
    if (mLastVSeq != vseq)
    {
      Int_t delta_t = (time - mOpenTime).GetSec();
      mIoInfo.mReqs.push_back(SXrdReq(mIoInfo.mLengthVec.size(), n_segments, total_length, delta_t));
    }
    else
    {
      mIoInfo.mReqs.back().IncLength(total_length);
      mIoInfo.mReqs.back().IncSubReqCount(n_segments);
    }
  }

  mExpectedReadVSegs = n_segments;
  mLastVSeq = vseq;

  // Ignore multi-file vector reads with several entries from the same file,
  // treat them as separate.
  // Would have to sum stuff up and commit it from end_read_vseg_if_expected()
  // but it seems I'd need one more state var (Bool_t bInReadV).
  AddVecReadSample(total_length / One_MB, n_segments);

  mLastMsgTime = time;
  Stamp(FID());
}

void XrdFile::RegisterFileClose(const GTime& close_time)
{
  end_read_vseg_if_expected();

  mCloseTime = close_time;
  Stamp(FID());
}
