// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_SVars_H
#define GledCore_SVars_H

#include <Rtypes.h>

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

template<typename T>
class SInertVar : public SMinMaxVar<T>
{
  typedef SMinMaxVar<T> MMV;

protected:
  T mDelta;

  T deltaminmax(T d) const
  {
    return d > mDelta ? mDelta : (d < -mDelta ? -mDelta : d);
  }
  T deltaminmax(T d, T dt) const
  {
    T lim = mDelta * dt;
    return d > lim ? lim : (d < -lim ? -lim : d);
  }

public:
  SInertVar()
    : MMV(), mDelta(1)
  {}

  SInertVar(T val, T min, T max, T max_delta) :
    MMV(val, min, max), mDelta(max_delta)
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

  T    GetDelta() const { return mDelta; }
  void SetDelta(T d)    { mDelta = d;    }

  void SetMinMaxDelta(T min, T max, T delta) { SetMinMax(min, max); mDelta = delta; }

  void Print() const
  {
    printf("%f [%f, %f; %f]\n", MMV::mVal, MMV::mMin, MMV::mMax, mDelta); 
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
  mutable T      mDesire;
  mutable Bool_t bDesireSatisfied;

public:
  SDesireVar() :
    IV(), mDesire(0), bDesireSatisfied(true)
  {}

  SDesireVar(T val, T min, T max, T delta) :
    IV(val, min, max, delta), mDesire(val), bDesireSatisfied(true)
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

  T    GetDesire()     const { return mDesire; }
  void SetDesire(T d)  const { bDesireSatisfied = false; mDesire = valminmax(d); }
  void DesireToValue() const { bDesireSatisfied = true;  mDesire = MMV::mVal; }

  T DeltaDesire(T d) const
  {
    bDesireSatisfied = false;
    mDesire = valminmax(mDesire + d);
    return mDesire;
  }

  Bool_t IsDesireSatisfied() const { return bDesireSatisfied; }

  Bool_t IsAboveDesire() const { return MMV::mVal > mDesire; }
  Bool_t IsBelowDesire() const { return MMV::mVal < mDesire; }

  void SetMinMaxDeltaDesire(T min, T max, T delta, T desire)
  {
    SetMinMaxDelta(min, max, delta);
    SetDesire(desire);
  }

  void TimeTick(T dt)
  {
    if (bDesireSatisfied)
      return;

    if (MMV::mVal < mDesire)
    {
      Delta(IV::mDelta, dt);
      if (MMV::mVal > mDesire)
      {
	MMV::mVal = mDesire;
	bDesireSatisfied = true;
      }
    }
    else if (MMV::mVal > mDesire)
    {
      Delta(-IV::mDelta, dt);
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
    printf("%f -> %f [%f, %f; %f]\n", MMV::mVal, mDesire, MMV::mMin, MMV::mMax, IV::mDelta); 
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
