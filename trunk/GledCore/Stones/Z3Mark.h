// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Z3Mark_H
#define Gled_Z3Mark_H

// Includes
#include <Stones/ZVector.h>

class Z3Mark {
protected:
  ZVector	mR;	// Mark
  ZVector	mS;	// Errors (or scales ...)
public:
  Z3Mark() : mR(3), mS(3) {}
  Z3Mark(Real_t x,Real_t y,Real_t z,Real_t u,Real_t v,Real_t w) : mR(3), mS(3) {
    mR(0u)=x; mR(1u)=y; mR(2u)=z; mS(0u)=u; mS(1u)=v; mS(2u)=w;
  }
  Z3Mark(const Z3Mark& p) : mR(p.mR), mS(p.mS) {}
  virtual ~Z3Mark() {}

  Real_t& R(Int_t i) { return mR(i); }
  Real_t& S(Int_t i) { return mS(i); }
  Real_t& R(UCIndex_t i) { return mR(i); }
  Real_t& S(UCIndex_t i) { return mS(i); }
  // const stuff
  const ZVector& R() const { return mR; }
  const ZVector& S() const { return mS; }
  Real_t R(Int_t i) const { return mR(i); }
  Real_t S(Int_t i) const { return mS(i); }
  Real_t R(UCIndex_t i) const { return mR(i); }
  Real_t S(UCIndex_t i) const { return mS(i); }

  ClassDef(Z3Mark, 1)
}; // endclass Z3Mark

ostream& operator<<(ostream& s, Z3Mark& m);

#include <vector>
typedef	vector<Z3Mark>			v3Mark_t;
typedef	vector<Z3Mark>::iterator	v3Mark_i;
typedef	vector<Z3Mark*>			vp3Mark_t;
typedef	vector<Z3Mark*>::iterator	vp3Mark_i;

#endif
