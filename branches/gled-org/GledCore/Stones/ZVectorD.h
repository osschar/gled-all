// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZVectorD
#define Gled_ZVectorD

#include <Gled/GledTypes.h>
#include <TVectorD.h>
class ZVector;

class ZVectorD : public TVectorD {
public:
  ZVectorD() : TVectorD(5) {}
  ZVectorD(Int_t n) : TVectorD(n) {}
  ZVectorD(Int_t l, Int_t u) : TVectorD(l, u) {}
  ZVectorD(const ZVectorD& v) : TVectorD(v) {}
  ZVectorD(const ZVectorD& v, UCIndex_t l, UCIndex_t u) : TVectorD(u-l+1) {
    for(UCIndex_t i=0; i<=u-l; i++) fElements[i]=v(i+l);
  }
  ZVectorD(ZVector& x);

  const Double_t& operator()(UCIndex_t index) const { return fElements[index]; }
  Double_t& operator()(UCIndex_t index) { return fElements[index]; }
  Double_t* GetArray() { return fElements; }

  const Double_t& operator()(Int_t index) const { return TVectorD::operator()(index); }
  Double_t& operator()(Int_t index) { return TVectorD::operator()(index); }

  ZVectorD& operator=(const ZVectorD& source);
  ZVectorD& operator=(const ZVector&  source);

  friend class ZVector;
  ClassDef(ZVectorD, 1)
}; // endclass ZVectorD

inline ZVectorD& ZVectorD::operator=(const ZVectorD& source)
{
  if(this==&source) return *this;
  if(!AreCompatible(*this, source)) {
    ResizeTo(source.GetNoElements());
  }
  memcpy(fElements, source.fElements, fNrows*sizeof(Double_t));
  return *this;
}

#endif
