// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZTrans_H
#define GledCore_ZTrans_H

// Includes
class ZNode;
class ZMark;
class TVectorF;

#include <TMatrixF.h>
#include <TMath.h>

// Should rewrite this using TVectorF (now that i have GetArray)
struct ZVec3 {
  Float_t v[3];
  ZVec3() { v[0] = v[1] = v[2] = 0; }
  ZVec3(Float_t x, Float_t y, Float_t z) { v[0]=x; v[1]=y; v[2]=z; }
  Float_t& operator()(Int_t i) { return v[i]; }
};

struct ZVec3D {
  Double_t v[3];
  ZVec3D() { v[0] = v[1] = v[2] = 0; }
  ZVec3D(Double_t x, Double_t y, Double_t z) { v[0]=x; v[1]=y; v[2]=z; }
  Double_t& operator()(Int_t i) { return v[i]; }
};

/**************************************************************************/
// ZTrans -- 3D transformation in generalised coordinates
/**************************************************************************/

class ZTrans : public TMatrixF {

  mutable Float_t	mA1;   //!
  mutable Float_t	mA2;   //!
  mutable Float_t	mA3;   //!
  mutable Bool_t	bAsOK; //!

  void _init();

public:
  ZTrans();
  ZTrans(const TMatrixF& m);
  ZTrans(const ZTrans& z);
  ZTrans(const ZNode* n);
  virtual ~ZTrans() {}

  // more like define ... to use insted of (*this)
  const ZTrans& ZT() const { return *this; }
  ZTrans&       ZT()       { return *this; }

  Int_t Set3Pos(Float_t x, Float_t y, Float_t z);
  Int_t SetPos(const TVectorF& v);
  Int_t SetPos(const ZTrans& t);
  ZVec3 Get3Pos() const
  { return ZVec3((*this)(1,4), (*this)(2,4), (*this)(3,4)); }

  TVectorF* GetZVector() const;
  TVectorF* GetBaseV(UInt_t b) const;

  void SetRot(Int_t i, Int_t j, Float_t f);
  void SetTrans(ZTrans& t);
  void SetBaseV(UInt_t i, Float_t x, Float_t y, Float_t z);

  // Space-Time Position
  // Move in LocalFrame; ai=1(fwd,bck), 2(up/down), 3(r/l)
  Int_t MoveLF(Int_t ai, Float_t amount=1);
  // Rotate in LocalFrame; i1, i2 ... generator indices
  Int_t RotateLF(Int_t i1, Int_t i2, Float_t amount=0.02);
  // Now in some other frame ... 
  Int_t Move(ZTrans* a, Int_t ai, Float_t amount=1);
  Int_t Rotate(ZTrans* a, Int_t i1, Int_t i2, Float_t amount=0.02);
  Int_t SetRotByAngles(Float_t a1, Float_t a2, Float_t a3);
  void  Scale(Float_t sx, Float_t sy, Float_t sz);
  ZVec3 Get3Rot() const;

  // Stuff to do w/ 3Vecs ...
  TVectorF& Mult3Vec(TVectorF& v) const;
  TVectorF& Rot3Vec(TVectorF& v) const;

  void Transpose();
  void Invert();

  static ZTrans UnitTrans;

  ClassDef(ZTrans, 1)
};

ostream& operator<<(ostream& s, const ZVec3& v);
ostream& operator<<(ostream& s, const ZTrans& t);

#endif
