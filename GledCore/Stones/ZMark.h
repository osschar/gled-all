// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZMark_H
#define GledCore_ZMark_H

// Includes
#include <TVectorF.h>

class ZMark {
protected:
  TVectorF	mX;	// Mark
  TVectorF	mS;	// Errors (or scales ...)
  // Perhaps will need ZTrans to make the bestiary complete
  // or in subclass ... ZSeed
public:
  ZMark() : mX(5), mS(5) { mX(4) = mS(4) = 1; mS(0) = 1; }
  ZMark(Float_t x,Float_t y,Float_t z,Float_t u,Float_t v,Float_t w) : mX(5), mS(5) {
    mX(1)=x; mX(2)=y; mX(3)=z; mS(1)=u; mS(2)=v; mS(3)=w;
    mX(4) = mS(4) = 1; mS(0) = 1;
  }
  ZMark(const ZMark& p) : mX(p.mX), mS(p.mS) {}
  virtual ~ZMark() {}

  Float_t& X(Int_t i) { return mX(i); }
  Float_t& S(Int_t i) { return mS(i); }
  // const
  const TVectorF& X() const { return mX; }
  const TVectorF& S() const { return mS; }
  Float_t X(Int_t i) const { return mX(i); }
  Float_t S(Int_t i) const { return mS(i); }

  ClassDef(ZMark, 1)
}; // endclass ZMark

ostream& operator<<(ostream& s, ZMark& m);

#endif
