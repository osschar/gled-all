// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GTime_H
#define GledCore_GTime_H

#include <Rtypes.h>

class GTime
{
public:
  enum Init_e { I_Now };

protected:
  Long_t	mSec;	// X{GS}
  Long_t	mMuSec;	// X{GS}

public:
  GTime(Long_t s=0, Long_t mu=0) : mSec(s), mMuSec(mu) {}
  GTime(Init_e /*i*/) { SetNow(); }
  GTime(const GTime& t) : mSec(t.mSec), mMuSec(t.mMuSec) {}

  ~GTime() {}

  static GTime Now() { return GTime(I_Now); }

  void  SetNow();
  GTime TimeUntilNow();

  Bool_t IsZero()    const { return mSec == 0 && mMuSec == 0; }
  Bool_t IsNonZero() const { return mSec != 0 || mMuSec != 0; }

  GTime& operator=(Long_t mus);
  GTime& operator=(ULong_t mus);
  GTime& operator=(Double_t sec);

  GTime& operator+=(const GTime& t);
  GTime& operator-=(const GTime& t);

  GTime& operator+=(Long_t mus);
  GTime& operator-=(Long_t mus);

  GTime& operator+=(ULong_t mus);
  GTime& operator-=(ULong_t mus);

  GTime& operator+=(Double_t sec);
  GTime& operator-=(Double_t sec);

  GTime operator+(const GTime& t) const;
  GTime operator-(const GTime& t) const;

  bool  operator<(const GTime& t) const;
  bool  operator>(const GTime& t) const;
  bool  operator<=(const GTime& t) const;
  bool  operator>=(const GTime& t) const;
  bool  operator==(const GTime& t) const;

  Double_t ToDouble()  { return mSec + 1e-6  * mMuSec; }
  Float_t  ToFloat()   { return mSec + 1e-6f * mMuSec; }
  ULong_t  ToMiliSec() { return mSec*1000 + mMuSec/1000; }
  ULong_t  ToMiliSec(Long_t max) { return mSec >= max ? max*1000 : mSec*1000 + mMuSec/1000; }

  void Sleep();

  static UInt_t SleepMiliSec(UInt_t ms,
			     Bool_t break_on_signal=true,
			     Bool_t warn_on_signal=true);

#include "GTime.h7"
  ClassDefNV(GTime, 1);
}; // endclass GTime

#endif