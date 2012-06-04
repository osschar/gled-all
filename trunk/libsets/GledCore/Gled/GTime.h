// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GTime_H
#define GledCore_GTime_H

#include <Rtypes.h>
#include <TString.h>

class TBuffer;

class GTime
{
public:
  enum Init_e { I_Zero, I_Now, I_Never };

protected:
  Long64_t	mSec;	// X{GS}
  Long64_t	mMuSec;	// X{GS}

  void canonize();

public:
  GTime() : mSec(0), mMuSec(0) {}
  explicit GTime(Init_e i);
  explicit GTime(Double_t s) { *this = s; }
  explicit GTime(Long64_t s, Long64_t mu) : mSec(s), mMuSec(mu) { canonize(); }
  GTime(const GTime& t) : mSec(t.mSec), mMuSec(t.mMuSec) {}

  ~GTime() {}

  static GTime Now()   { return GTime(I_Now); }
  static GTime Never() { return GTime(I_Never); }
  static GTime MiliSec(Long64_t ms) { return GTime(0ll, 1000ll * ms); }

  void   SetNow();
  GTime  TimeUntilNow();

  void   SetZero()         { mSec = mMuSec = 0; }
  Bool_t IsZero()    const { return mSec == 0 && mMuSec == 0; }
  Bool_t IsNonZero() const { return mSec != 0 || mMuSec != 0; }

  void   SetNever();
  Bool_t IsNever() const;

  GTime& operator=(Double_t sec);

  GTime& operator+=(const GTime& t);
  GTime& operator-=(const GTime& t);

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

  TString  ToAscUTC  (Bool_t show_tz=true) const;
  TString  ToAscLocal(Bool_t show_tz=true) const;
  TString  ToDateTimeUTC  (Bool_t show_tz=true) const;
  TString  ToDateTimeLocal(Bool_t show_tz=true) const;
  TString  ToCompactDateTimeUTC  () const;
  TString  ToCompactDateTimeLocal() const;
  TString  ToDateUTC  () const;
  TString  ToDateLocal() const;
  TString  ToWebTimeGMT  (Bool_t show_tz=true) const;
  TString  ToWebTimeLocal(Bool_t show_tz=true) const;
  TString  ToHourMinSec(Bool_t force_non_negative=false) const;

  void     Sleep();

  static Long64_t SleepMiliSec(UInt_t ms,
			       Bool_t break_on_signal=true,
			       Bool_t warn_on_signal=true);

  void  NetStreamer(TBuffer& b);
  Int_t NetBufferSize() const { return 2 * sizeof(Long64_t); }

#include "GTime.h7"
  ClassDefNV(GTime, 1);
}; // endclass GTime

#endif
