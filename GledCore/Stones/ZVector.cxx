// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZVector.h"
ClassImp(ZVector)

#include <Stones/ZVectorD.h>

ZVector::ZVector(ZVectorD& x) : TVector(x.GetNrows()) {
  *this = x;
}

Double_t ZVector::Dot(const ZVector& x, UInt_t M) const
{
  if(M==0) M = GetNrows() <? x.GetNrows();
  Double_t r=0;
  for(UInt_t i=0; i<M; ++i) r += (*this)(i)*x(i);
  return r;
}

Double_t ZVector::Norm(UInt_t M)
{
  if(M==0) M = GetNrows();
  Double_t ni = 1/TMath::Sqrt(Norm2Sqr()); 
  for(UInt_t i=0; i<M; ++i) (*this)(i) *= ni;
  return ni;
}

Double_t ZVector::OrtoNorm(const ZVector& x, UInt_t M)
{
  if(M==0) M = GetNrows();
  Double_t dp = Dot(x, M);
  for(UInt_t i=0; i<M; ++i) (*this)(i) -= x(i)*dp;
  Double_t ni = 1/TMath::Sqrt(Norm2Sqr()); 
  for(UInt_t i=0; i<M; ++i) (*this)(i) *= ni;
  return dp;
}

ZVector& ZVector::operator=(const ZVectorD& source)
{
  if(GetNrows() != source.GetNrows())
    ResizeTo(source.GetNrows());
  for(UCIndex_t i=0; i<GetNrows(); i++)
    fElements[i] = source(i);
  return *this;
}
