// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Z3Point.h"
#include <Stones/Z3Mark.h>
#include <Stones/ZTrans.h>

ClassImp(Z3Point)

static const TMatrixF Z3PointMatrixProto(3,3);

/**************************************************************************/

void Z3Point::_init(const Z3Mark& m, bool Ss_are_sigmas)
{
  // if Ss_are_sigmas builds inverse of covariance matrix (\1/sigma_x^2 ... \)
  for(Int_t i=0; i<=2; i++) {
    mR(i) = m.R(i);
    if(Ss_are_sigmas)
      // watchit !!!! 0 means infinite error !!!!
      mW(i,i) = (m.S(i)==0) ? 0 : 1/(m.S(i)*m.S(i));
    else
      mW(i,i) = m.S(i);
  }
}

/**************************************************************************/

Z3Point::Z3Point() : mR(3), mW(TMatrix::kUnit, Z3PointMatrixProto) {}

Z3Point::Z3Point(const Z3Point& z) : mR(z.mR), mW(z.mW) {}

Z3Point::Z3Point(const Z3Mark& m, bool Ss_are_sigmas) :
  mR(3), mW(TMatrix::kUnit, Z3PointMatrixProto)
{ _init(m, Ss_are_sigmas); }

Z3Point::Z3Point(const Z3Mark& m, const ZTrans& t, bool Ss_are_sigmas) :
  mR(3), mW(TMatrix::kUnit, Z3PointMatrixProto)
{ _init(m, Ss_are_sigmas); Transform(t); }

/**************************************************************************/

void Z3Point::Transform(const ZTrans& t)
{
  // do tWt^-1, tR w/ 3x3 subset of t
  TMatrix tmp(TMatrix::kZero, Z3PointMatrixProto);
  // W t^-1 ... t^-1 is transpose of t (as ZTrans should be O(4))
  for(Int_t i=0; i<3; i++)
    for(Int_t j=0; j<3; j++)
      for(Int_t k=0; k<3; k++)
	tmp(i,j) += mW(i,k) * t(j+1u,k+1u);
  TVectorF vmp(mR);
  for(Int_t i=0; i<3; i++) {
    mR(i) = t(i+1,4u); // Init pos w/ parental move
    for(Int_t j=0; j<3; j++) {
      mW(i,j) = 0;
      for(Int_t k=0; k<3; k++)
	mW(i,j) += t(i+1u,k+1u) * tmp(k,j);
      mR(i) += t(i+1u,j+1u) * vmp(j);
    }
  }
}

/**************************************************************************/

#include <iomanip>
using namespace std;

ostream& operator<<(ostream& s, const Z3Point& p) {
  s.setf(ios::fixed, ios::floatfield); s.precision(3);
  for(Int_t i=0; i<3; i++) {
    s << p.R(i) << "\t|\t";
    for(Int_t j=0; j<3; j++)
      s << p.W(i,j) << ((j==2) ? "\n" : "\t");
  }
  return s;
}
