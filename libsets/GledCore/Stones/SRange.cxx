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
