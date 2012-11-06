// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SXrdIoInfo.h"

// SXrdIoInfo

//______________________________________________________________________________
//
//

ClassImp(SXrdIoInfoFiller);

//==============================================================================

SXrdIoInfoFiller::SXrdIoInfoFiller(SXrdIoInfo& ioi) :
  mIoI(ioi),
  mExpectedReadVSegs(0),
  mLastVSeq(0),
  bInReadV(false)
{}

//==============================================================================

void SXrdIoInfoFiller::begin_readv(Int_t n_segments, Int_t total_length, Int_t time, UChar_t vseq)
{
  // create record,
}

void SXrdIoInfoFiller::extend_readv(Int_t n_segments, Int_t total_length)
{
  // add totals
}

void SXrdIoInfoFiller::end_readv()
{
  // commit missing segs, if any and reset
}

//==============================================================================

void SXrdIoInfoFiller::RegisterRead(Long64_t offset, Int_t length, Int_t time)
{
  // error if mExpectedReadVSegs > 0 ... when I'm getting them
  if (bInReadV) end_readv();
  // XXXX Hmmh ... wouldn't this be valid readv record??? Argh, etc. But yes ... it should.

  mIoI.mReqs.push_back(SXrdReq(offset, length, time));
}

void SXrdIoInfoFiller::RegisterWrite(Long64_t offset, Int_t length, Int_t time)
{
  // error if mExpectedReadVSegs > 0 ... when I'm getting them
  if (bInReadV)
  {
    if (mExpectedReadVSegs)
      end_readv();
  }

  // Support both length notations.
  if (length > 0) length = -length;
  mIoI.mReqs.push_back(SXrdReq(offset, length, time));
}

//------------------------------------------------------------------------------

void SXrdIoInfoFiller::RegisterReadV(Int_t n_segments, Int_t total_length, Int_t time, UChar_t vseq)
{
  // ReadV record, unpacked info is not following.
  // Additional records with the same vseq are possible.
}

void SXrdIoInfoFiller::RegisterReadU(Int_t n_segments, Int_t total_length, Int_t time, UChar_t vseq)
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
}

void SXrdIoInfoFiller::RegisterReadVSeg(Long64_t offset, Int_t length)
{

}
