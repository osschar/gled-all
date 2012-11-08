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
//

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
  bInReadV  = false;

}

XrdFile::XrdFile(const Text_t* n, const Text_t* t) :
  ZGlass(n, t)
{
  _init();
}

XrdFile::~XrdFile()
{}

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
  // XXX if in readv and expecting segements, this is readv detail
  if (bInReadV && mExpectedReadVSegs > 0)
  {
    // XXXX if this below is 0 -- readv details end ... but can get another
    // batch with same vseq later!
    --mExpectedReadVSegs;
    return;
  }

  // !!!! delta_t for open time
  // mIoInfo.mReqs.push_back(SXrdReq(offset, length, time));

  AddReadSample(length / One_MB);
  SetLastMsgTime(time);
}

void XrdFile::RegisterWrite(Long64_t offset, Int_t length, const GTime& time)
{
  // XXXX this needs work
  // error if mExpectedReadVSegs > 0 ... when I'm getting them
  if (bInReadV)
  {
    if (mExpectedReadVSegs)
      end_readv();
  }

  // Make sure length is negative.
  if (length > 0) length = -length;

  // !!!! delta_t
  // mIoInfo.mReqs.push_back(SXrdReq(offset, length, time));
  AddWriteSample(-length / One_MB);
  SetLastMsgTime(time);
}

//------------------------------------------------------------------------------

void XrdFile::RegisterReadV(Int_t n_segments, Int_t total_length, const GTime& time, UChar_t vseq)
{
  // ReadV record, unpacked info is not following.
  // Additional records with the same vseq are possible.

  // ---

  AddVecReadSample(total_length / One_MB, n_segments);
  SetLastMsgTime(time);
}

void XrdFile::RegisterReadU(Int_t n_segments, Int_t total_length, const GTime& time, UChar_t vseq)
{
  // ReadV record, unpacked info is about to follow.
  // Additional records with the same vseq are possible but only after individual reads.

  // Wait ... this guy could call RegsterReadV ... argh.

  if (bInReadV)
  {
    if  (vseq == mLastVSeq)
    {
      // append to existing record
    }
    else
    {
      // error
      end_readv();
      // create new record
      bInReadV = true;
    }
  }
  else
  {
    mLastVSeq = vseq;
  }

  if (bInReadV) end_readv();

  // ---

  AddVecReadSample(total_length / One_MB, n_segments);
  SetLastMsgTime(time);
}

void XrdFile::RegisterReadVSeg(Long64_t offset, Int_t length)
{
  // XXXX This function could / should be protected, too ... like below,
  // if these stay.
}

void XrdFile::RegisterFileClose()
{
  // XXXX What's with this guy? There are times when I don't get clean close.
  // Could call it from place where data is put into tree.
  // Several invocations should not hurt.
  // Problematic only if readu was one the last message and some packets got lost.

  // A special function for close-stamp ... that also calls SetCloseTime().
  // I think now i call SetCloseTime manually from several places ... check!
}

//==============================================================================

// XXXX Not sure if I need those ...

void XrdFile::begin_readv(Int_t n_segments, Int_t total_length, Int_t time, UChar_t vseq)
{
  // create record,
}

void XrdFile::extend_readv(Int_t n_segments, Int_t total_length)
{
  // add totals
}

void XrdFile::end_readv()
{
  // commit missing segs, if any and reset
}
