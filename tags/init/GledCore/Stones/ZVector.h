// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZVector
#define Gled_ZVector

#include <Gled/GledTypes.h>
#include <TVector.h>
class ZVectorD;

class ZVector : public TVector {
public:
  ZVector() : TVector(5) {}
  ZVector(Int_t n) : TVector(n) {}
  ZVector(Int_t l, Int_t u) : TVector(l, u) {}
  ZVector(const ZVector& v) : TVector(v) {}
  ZVector(const ZVector& v, UCIndex_t l, UCIndex_t u) : TVector(u-l+1) {
    for(UCIndex_t i=0; i<=u-l; i++) fElements[i]=v(i+l);
  }
  ZVector(ZVectorD& x);

  Double_t Dot(const ZVector& x, UInt_t M=0) const;
  Double_t Norm(UInt_t M=0);
  Double_t OrtoNorm(const ZVector& x, UInt_t M=0);

  Real_t  operator()(UCIndex_t index) const { return fElements[index]; }
  Real_t& operator()(UCIndex_t index) { return fElements[index]; }
  Real_t* GetArray() { return fElements; }

  Real_t  operator()(Int_t index) const { return TVector::operator()(index); }
  Real_t& operator()(Int_t index) { return TVector::operator()(index); }

  ZVector& operator=(const ZVector&  source);
  ZVector& operator=(const ZVectorD& source);

  friend class ZVectorD;
  ClassDef(ZVector, 1)
}; // endclass ZVector

inline ZVector& ZVector::operator=(const ZVector &source)
{
  if(this==&source) return *this;
  if(!AreCompatible(*this, source)) {
    ResizeTo(source.GetNoElements());
  }
  memcpy(fElements, source.fElements, fNrows*sizeof(Real_t));
  return *this;
}

#endif
