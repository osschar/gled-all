// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZMark_
#define Gled_ZMark_

// Includes
#include <Stones/ZVector.h>

class ZMark {
protected:
  ZVector	mX;	// Mark
  ZVector	mS;	// Errors (or scales ...)
  // Perhaps will need ZTrans to make the bestiary complete
  // or in subclass ... ZSeed
public:
  ZMark() : mX(5), mS(5) { mX(4u) = mS(4u) = 1; mS(0u) = 1; }
  ZMark(Real_t x,Real_t y,Real_t z,Real_t u,Real_t v,Real_t w) : mX(5), mS(5) {
    mX(1u)=x; mX(2u)=y; mX(3u)=z; mS(1u)=u; mS(2u)=v; mS(3u)=w;
    mX(4u) = mS(4u) = 1; mS(0u) = 1;
  }
  ZMark(const ZMark& p) : mX(p.mX), mS(p.mS) {}
  virtual ~ZMark() {}

  Real_t& X(Int_t i) { return mX(i); }
  Real_t& S(Int_t i) { return mS(i); }
  Real_t& X(UCIndex_t i) { return mX(i); }
  Real_t& S(UCIndex_t i) { return mS(i); }
  // const
  const ZVector& X() const { return mX; }
  const ZVector& S() const { return mS; }
  Real_t X(Int_t i) const { return mX(i); }
  Real_t S(Int_t i) const { return mS(i); }
  Real_t X(UCIndex_t i) const { return mX(i); }
  Real_t S(UCIndex_t i) const { return mS(i); }

  ClassDef(ZMark, 1)
}; // endclass ZMark

ostream& operator<<(ostream& s, ZMark& m);

#include <vector>

typedef	vector<ZMark>		vMark_t;
typedef	vector<ZMark>::iterator	vMark_i;

#endif
