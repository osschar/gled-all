// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Z3Mark_H
#define Gled_Z3Mark_H

// Includes
#include <TVectorF.h>

class Z3Mark {
protected:
  TVectorF	mR;	// Mark
  TVectorF	mS;	// Errors (or scales ...)
public:
  Z3Mark() : mR(3), mS(3) {}
  Z3Mark(Float_t x,Float_t y,Float_t z,Float_t u,Float_t v,Float_t w) : mR(3), mS(3) {
    mR(0u)=x; mR(1u)=y; mR(2u)=z; mS(0u)=u; mS(1u)=v; mS(2u)=w;
  }
  Z3Mark(const Z3Mark& p) : mR(p.mR), mS(p.mS) {}
  virtual ~Z3Mark() {}

  Float_t& R(Int_t i) { return mR(i); }
  Float_t& S(Int_t i) { return mS(i); }
  // const stuff
  const TVectorF& R() const { return mR; }
  const TVectorF& S() const { return mS; }
  Float_t R(Int_t i) const { return mR(i); }
  Float_t S(Int_t i) const { return mS(i); }

  ClassDef(Z3Mark, 1)
}; // endclass Z3Mark

ostream& operator<<(ostream& s, Z3Mark& m);

#endif
