// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// SRange
//
//

#include "SRange.h"

#include <TMath.h>

ClassImp(SRange);

//==============================================================================

SRange::SRange()
{
  Reset();
}

void SRange::Reset()
{
  mMin = mMax = mSumX = mSumX2 = 0;
  mN = 0;
}

void SRange::Reset(Double_t min, Double_t max, Double_t sumx, Double_t sumx2, ULong64_t n)
{
  mMin  = min;  mMax   = max;
  mSumX = sumx; mSumX2 = sumx2;
  mN    = n;
}

void SRange::SetSumX2FromSigma(Double_t sigma)
{
  if (mN > 0)
    mSumX2 = mN*sigma*sigma + mSumX*mSumX/mN;
  else
    mSumX2 = 0;
}

//------------------------------------------------------------------------------

void SRange::AddSample(Double_t x)
{
  if (mN == 0) {
    mMin = mMax = x;
  } else {
    if (x < mMin) mMin = x;
    if (x > mMax) mMax = x;
  }
  mSumX  += x;
  mSumX2 += x*x;
  ++mN;
}

//------------------------------------------------------------------------------

Double_t SRange::GetAverage() const
{
  if (mN > 0)
    return mSumX / mN;
  else
    return 0;
}

Double_t SRange::GetSigma() const
{
  if (mN > 0)
    return TMath::Sqrt((mSumX2 - mSumX*mSumX/mN)/mN);
  else
    return 0;
}

//------------------------------------------------------------------------------

void SRange::Dump(const TString& prefix, const TString& postfix) const
{
  printf("%s%5lld : (%10.3f, %10.3f, %10.3f) : %10.3f%s",
         prefix.Data(),
         mN, mMin, GetAverage(), mMax, GetSigma(),
         postfix.Data());
}
