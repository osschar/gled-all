// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Z3Point_H
#define Gled_Z3Point_H

// Includes
class Z3Mark;
class ZTrans;
#include <TVectorF.h>
#include <TMatrixF.h>

class Z3Point {
private:
  void _init(const Z3Mark& m, bool Ss_are_sigmas=false);

protected:
  TVectorF	mR;
  TMatrixF	mW;

public:
  Z3Point();
  Z3Point(const Z3Point& z);
  Z3Point(const Z3Mark& m, bool Ss_are_sigmas=false);
  Z3Point(const Z3Mark& m, const ZTrans& t, bool Ss_are_sigmas=false);
  // Here could take advantage of diagonal structure of W
  virtual ~Z3Point() {}

  Float_t& R(Int_t i) { return mR(i); }
  Float_t& W(Int_t i, Int_t j) { return mW(i,j); }
  // const
  const TVectorF& R() const { return mR; }
  const TMatrixF& W() const { return mW; }
  Float_t R(Int_t i) const { return mR(i); }
  Float_t W(Int_t i, Int_t j) const { return mW(i,j); }
  
  void Transform(const ZTrans& t);

  ClassDef(Z3Point, 1)
}; // endclass Z3Point

ostream& operator<<(ostream& s, const Z3Point& p);

#endif
