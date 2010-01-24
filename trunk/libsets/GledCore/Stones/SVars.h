// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_SVars_H
#define GledCore_SVars_H

#include <TMath.h>

//==============================================================================
// SVars
//==============================================================================

template<typename T>
class SMinMaxVar
{
protected:
  T mVal, mMin, mMax;

  T valminmax(T v) const
  {
    return v > mMax ? mMax : (v < mMin ? mMin : v);
  }

public:
  SMinMaxVar()
    : mVal(0), mMin(0), mMax(1)
  {}

  SMinMaxVar(T val, T min, T max) :
    mVal(val), mMin(min), mMax(max)
  {}

  operator T() const { return mVal; }
  T operator=(T v) { return Set(v); }

  T Get() const { return mVal; }

  T Set(T v)
  {
    // Returns the actual value set.
    mVal = valminmax(v);
    return mVal;
  }

  T Delta(T d)
  {
    // Returns the actual delta used.
    T ov = mVal;
    mVal = valminmax(mVal + d);
    return mVal - ov;
  }

  T    GetMin() const { return mMin; }
  T    GetMax() const { return mMax; }
  // Check min/max mismatch?
  void SetMin(T min) { mMin = min; }
  void SetMax(T max) { mMax = max; }  

  void SetMinMax(T min, T max) { mMin = min; mMax = max; }

  void Print() const
  {
    printf("%f [%f, %f]\n", mVal, mMin, mMax); 
  }

  ClassDefNV(SMinMaxVar, 1);
};

typedef SMinMaxVar<Float_t>  SMinMaxVarF;
typedef SMinMaxVar<Double_t> SMinMaxVarD;

// Needed this in HTrans
// #ifndef __APPLE__
// template class SMinMaxVar<Float_t>;
// template class SMinMaxVar<Double_t>;
// #endif


//==============================================================================
// SInertVar
//==============================================================================

// Inert variable with limited maximum change (per unit time).
// Two deltas must be specified:
//   1. DeltaInc - applied when absolute value is increasing,
//   2. DeltaDec - applied when absolute value is decreasing.
// They should both be positive.

template<typename T>
class SInertVar : public SMinMaxVar<T>
{
  typedef SMinMaxVar<T> MMV;

protected:
  T mDeltaInc, mDeltaDec;

  T deltaminmax(T d) const
  {
    if (d > 0)
    {
      T lim = MMV::mVal >= 0 ? mDeltaInc : mDeltaDec;
      return d > lim ? lim : d;
    } else {
      T lim = MMV::mVal <= 0 ? -mDeltaInc : -mDeltaDec;
      return d < lim ? lim : d;
    }
  }

  T deltaminmax(T d, T dt) const
  {
    if (d > 0)
    {
      T lim = MMV::mVal >= 0 ?  mDeltaInc*dt :  mDeltaDec*dt;
      return d > lim ? lim : d;
    } else {
      T lim = MMV::mVal <= 0 ? -mDeltaInc*dt : -mDeltaDec*dt;
      return d < lim ? lim : d;
    }
  }

  T deltamaxpos(T dt) const
  {
    return MMV::mVal >= 0 ?  mDeltaInc*dt :  mDeltaDec*dt;
  }

  T deltamaxneg(T dt) const
  {
    return MMV::mVal <= 0 ? -mDeltaInc*dt : -mDeltaDec*dt;
  }

public:
  SInertVar()
    : MMV(), mDeltaInc(1), mDeltaDec(-1)
  {}

  SInertVar(T val, T min, T max, T delta) :
    MMV(val, min, max), mDeltaInc(delta), mDeltaDec(-delta)
  {}

  SInertVar(T val, T min, T max, T delta_i, T delta_d) :
    MMV(val, min, max), mDeltaInc(delta_i), mDeltaDec(delta_d)
  {}

  T operator=(T v) { return Set(v); }

  T Set(T v)
  {
    T d = deltaminmax(v - MMV::mVal);
    return MMV::Set(MMV::mVal + d);
  }

  T Delta(T d)
  {
    d = deltaminmax(d);
    return MMV::Delta(d);
  }

  T Delta(T d, T dt)
  {
    d = deltaminmax(d, dt);
    return MMV::Delta(d);
  }

  T DeltaMaxPositive(T dt)
  {
    return MMV::Delta(deltamaxpos(dt));
  }

  T DeltaMaxNegative(T dt)
  {
    return MMV::Delta(deltamaxneg(dt));
  }

  T DeltaMax(Int_t sign, T dt)
  {
    if      (sign > 0) return DeltaMaxPositive(dt);
    else if (sign < 0) return DeltaMaxNegative(dt);
    else               return 0;
  }

  T    GetDeltaInc() const { return mDeltaInc; }
  void SetDeltaInc(T d)    { mDeltaInc = d;    }
  T    GetDeltaDec() const { return mDeltaDec; }
  void SetDeltaDec(T d)    { mDeltaDec = d;    }

  void SetMinMaxDelta(T min, T max, T delta_i, T delta_d)
  { SetMinMax(min, max); mDeltaInc = delta_i; mDeltaDec = delta_d; }

  void Print() const
  {
    printf("%f [%f, %f; %f, %f]\n", MMV::mVal, MMV::mMin, MMV::mMax, mDeltaInc, mDeltaDec); 
  }

  ClassDefNV(SInertVar, 1);
};

typedef SInertVar<Float_t>  SInertVarF;
typedef SInertVar<Double_t> SInertVarD;


//==============================================================================
// SDesireVar
//==============================================================================

template<typename T>
class SDesireVar : public SInertVar<T>
{
  typedef SMinMaxVar<T> MMV;
  typedef SInertVar<T>  IV;

protected:
  T              mStdDesireDelta;  // Standard delta of desire.
  mutable T      mDesire;
  mutable Bool_t bDesireSatisfied;

public:
  SDesireVar() :
    IV(), mStdDesireDelta(1), mDesire(0), bDesireSatisfied(true)
  {}

  SDesireVar(T val, T min, T max, T delta) :
    IV(val, min, max, delta),
    mStdDesireDelta(delta),
    mDesire(val), bDesireSatisfied(true)
  {}

  SDesireVar(T val, T min, T max, T delta_i, T delta_d) :
    IV(val, min, max, delta_i, delta_d),
    mStdDesireDelta(TMath::Max(delta_i, delta_d)),
    mDesire(val), bDesireSatisfied(true)
  {}

  T operator=(T v) { return Set(v); }

  T Set(T v)
  {
    bDesireSatisfied = false;
    T d = deltaminmax(v - MMV::mVal);
    return MMV::Set(MMV::mVal + d);
  }

  T Delta(T d)
  {
    bDesireSatisfied = false;
    d = deltaminmax(d);
    return MMV::Delta(d);
  }

  T Delta(T d, T dt)
  {
    bDesireSatisfied = false;
    d = deltaminmax(d, dt);
    return MMV::Delta(d);
  }

  T    GetStdDesireDelta() const { return mStdDesireDelta; }
  void SetStdDesireDelta(T sdd)  { mStdDesireDelta = sdd; }

  T    GetDesire()     const { return mDesire; }
  void SetDesire(T d)  const { bDesireSatisfied = false; mDesire = valminmax(d); }
  void DesireToValue() const { bDesireSatisfied = true;  mDesire = MMV::mVal; }

  T DeltaDesire(T d) const
  {
    bDesireSatisfied = false;
    mDesire = valminmax(mDesire + d);
    return mDesire;
  }

  T DeltaDesireMax(Int_t ifac, T dt) const
  {
    if (ifac) return DeltaDesire(ifac*mStdDesireDelta*dt);
    else      return 0;
  }

  Bool_t IsDesireSatisfied() const { return bDesireSatisfied; }

  Bool_t IsAboveDesire() const { return MMV::mVal > mDesire; }
  Bool_t IsBelowDesire() const { return MMV::mVal < mDesire; }

  void SetMinMaxDeltaDesire(T min, T max, T delta_i, T delta_d, T desire)
  {
    SetMinMaxDelta(min, max, delta_i, delta_d);
    SetDesire(desire);
  }

  void TimeTick(T dt)
  {
    if (bDesireSatisfied)
      return;

    if (MMV::mVal < mDesire)
    {
      DeltaMaxPositive(dt);
      if (MMV::mVal > mDesire)
      {
	MMV::mVal = mDesire;
	bDesireSatisfied = true;
      }
    }
    else if (MMV::mVal > mDesire)
    {
      DeltaMaxNegative(dt);
      if (MMV::mVal < mDesire)
      {
	MMV::mVal = mDesire;
	bDesireSatisfied = true;
      }
    }
    else
    {
      bDesireSatisfied = true;
    }
  }

  void Print() const
  {
    printf("%f -> %f [%f, %f; %f, %f]\n", MMV::mVal, mDesire,
	   MMV::mMin, MMV::mMax, IV::mDeltaInc, IV::mDeltaDec); 
  }

  ClassDefNV(SDesireVar, 1);
}; // endclass SDesireVar

typedef SDesireVar<Float_t>  SDesireVarF;
typedef SDesireVar<Double_t> SDesireVarD;


//==============================================================================
// SExpDesireVar
//==============================================================================

template<typename T>
class SExpDesireVar : public SDesireVar<T>
{
  typedef SMinMaxVar<T> MMV;
  typedef SInertVar<T>  IV;
  typedef SDesireVar<T> DV;

protected:
  T       mOmega;   // factor for difference of value
  UChar_t mExpMode; // reference modes for inc / dec (min, max, zero or desire) - separate inc / dec?

public:
  SExpDesireVar()
    : DV(), mOmega(0), mExpMode(0)
  {}

  SExpDesireVar(T val, T min, T max, T delta) :
    DV(val, min, max, delta), mOmega(0), mExpMode(0)
  {}

  SExpDesireVar(T val, T min, T max, T delta, T omega, T mode) :
    DV(val, min, max, delta), mOmega(omega), mExpMode(mode)
  {}

  // This is mostly an idea ... will implement when needed.
  // Guess soon, as seems reasonable for engine throttle. [3.10.2009]

  /*
  T operator=(T v) { return Set(v); }

  void Print() const
  {
    printf("%f -> %f [%f, %f; %f]\n", MMV::mVal, mDesire, MMV::mMin, MMV::mMax, IV::mDelta); 
  }
  */

  ClassDefNV(SExpDesireVar, 1);
}; // endclass SExpDesireVar

typedef SExpDesireVar<Float_t>  SExpDesireVarF;
typedef SExpDesireVar<Double_t> SExpDesireVarD;

#endif
