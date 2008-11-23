// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GTime_H
#define GledCore_GTime_H

#include <Rtypes.h>

class GTime {
public:
  enum Init_e { I_Now };

protected:
  Long_t	mSec;	// X{GS}
  Long_t	mMuSec;	// X{GS}

public:
  GTime(Long_t s=0, Long_t mu=0) : mSec(s), mMuSec(mu) {}
  virtual ~GTime() {}
  GTime(Init_e i) { SetNow(); }
  GTime(const GTime& t) : mSec(t.mSec), mMuSec(t.mMuSec) {}
  static GTime Now() { return GTime(I_Now); }

  void  SetNow();
  GTime TimeUntilNow();

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

  Double_t ToDouble()  { return mSec + 1e-6*mMuSec; }
  ULong_t  ToMiliSec() { return mSec*1000 + mMuSec/1000; }

  void Sleep();

  static void SleepMiliSec(UInt_t ms);

#include "GTime.h7"
  ClassDef(GTime, 1);
}; // endclass GTime

#endif
