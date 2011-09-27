// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GTS_SRange_H
#define GTS_SRange_H

#include <Rtypes.h>

class SRange
{
protected:
  Double_t	mMin;   // X{GS}
  Double_t	mMax;   // X{GS}
  Double_t      mSumX;  // X{GS}
  Double_t      mSumX2; // X{GS}
  UInt_t        mN;     // X{GS}

public:
  SRange();

  void Reset();
  void AddSample(Double_t x);

  Double_t GetAverage() const;
  Double_t GetSigma() const;

#include "SRange.h7"
  ClassDefNV(SRange, 1);
}; // endclass SRange

#endif
