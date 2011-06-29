// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GTime_H
#define GledCore_GTime_H

#include <Rtypes.h>
#include <TString.h>

class GTime
{
public:
  enum Init_e { I_Now };

protected:
  Long64_t	mSec;	// X{GS}
  Long64_t	mMuSec;	// X{GS}

public:
  GTime(Long64_t s=0, Long64_t mu=0) : mSec(s), mMuSec(mu) {}
  GTime(Init_e /*i*/) { SetNow(); }
  GTime(const GTime& t) : mSec(t.mSec), mMuSec(t.mMuSec) {}

  ~GTime() {}

  static GTime Now() { return GTime(I_Now); }

  void   SetNow();
  GTime  TimeUntilNow();

  Bool_t IsZero()    const { return mSec == 0 && mMuSec == 0; }
  Bool_t IsNonZero() const { return mSec != 0 || mMuSec != 0; }

  GTime& operator=(Long64_t mus);
  GTime& operator=(ULong64_t mus);
  GTime& operator=(Double_t sec);

  GTime& operator+=(const GTime& t);
  GTime& operator-=(const GTime& t);

  GTime& operator+=(Long64_t mus);
  GTime& operator-=(Long64_t mus);

  GTime& operator+=(ULong64_t mus);
  GTime& operator-=(ULong64_t mus);

  GTime& operator+=(Double_t sec);
  GTime& operator-=(Double_t sec);

  GTime  operator+(const GTime& t) const;
  GTime  operator-(const GTime& t) const;

  bool   operator<(const GTime& t) const;
  bool   operator>(const GTime& t) const;
  bool   operator<=(const GTime& t) const;
  bool   operator>=(const GTime& t) const;
  bool   operator==(const GTime& t) const;

  Double_t ToDouble()  const { return mSec + 1e-6  * mMuSec; }
  Float_t  ToFloat()   const { return mSec + 1e-6f * mMuSec; }
  Long64_t ToMiliSec() const { return mSec*1000 + mMuSec/1000; }
  Long64_t ToMiliSec(Long64_t max) const { return mSec >= max ? max*1000 : mSec*1000 + mMuSec/1000; }

  TString  ToAscGMT  (Bool_t show_tz=true) const;
  TString  ToAscLocal(Bool_t show_tz=true) const;

  void     Sleep();

  static Long64_t SleepMiliSec(UInt_t ms,
			       Bool_t break_on_signal=true,
			       Bool_t warn_on_signal=true);

#include "GTime.h7"
  ClassDefNV(GTime, 1);
}; // endclass GTime

#endif
