// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZTrans_H
#define GledCore_ZTrans_H

// Includes
class ZNode;
class ZMark;

#include <TVector3.h>
#include <TVectorF.h>
#include <TMatrixF.h>
#include <TMath.h>

// Should use TVector3!

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

protected:
  Float_t norm3_column(Int_t col);
  Float_t orto3_column(Int_t col, Int_t ref);

public:
  ZTrans();
  ZTrans(const TMatrixF& m);
  ZTrans(const ZTrans& z);
  ZTrans(const ZNode* n);
  virtual ~ZTrans() {}

  void  UnitTrans();
  Int_t Set3Pos(Float_t x, Float_t y, Float_t z);
  Int_t SetPos(const TVectorF& v);
  Int_t SetPos(const ZTrans& t);
  ZVec3 Get3Pos() const
  { return ZVec3((*this)(1,4), (*this)(2,4), (*this)(3,4)); }

  TVectorF* GetZVector() const;
  TVectorF* GetBaseV(Int_t b) const;
  TVector3  GetBaseVec3(Int_t b) const;
  TVector3  GetPosVec3() const           { return GetBaseVec3(4); }

  void SetRot(Int_t i, Int_t j, Float_t f);
  void SetTrans(const ZTrans& t);
  void SetBaseV(Int_t i, Float_t x, Float_t y, Float_t z);
  void SetBaseVec3(Int_t i, const TVector3& v);

  // Space-Time Position
  // Move in LocalFrame; ai=1(fwd,bck), 2(up/down), 3(r/l)
  Int_t MoveLF(Int_t ai, Float_t amount=1);
  Int_t Move3(Float_t x, Float_t y, Float_t z);
  // Rotate in LocalFrame; i1, i2 ... generator indices
  Int_t RotateLF(Int_t i1, Int_t i2, Float_t amount=0.02);

  // Now in some other frame ... 
  Int_t Move(ZTrans* a, Int_t ai, Float_t amount=1);
  Int_t Rotate(ZTrans* a, Int_t i1, Int_t i2, Float_t amount=0.02);
  Int_t SetRotByAngles(Float_t a1, Float_t a2, Float_t a3);
  ZVec3 Get3Rot() const;

  void     Scale3(Float_t sx, Float_t sy, Float_t sz);
  void     GetScale3(Float_t& sx, Float_t& sy, Float_t& sz);
  void     Unscale3(Float_t& sx, Float_t& sy, Float_t& sz);
  Float_t  Unscale3();

  // Stuff to do w/ 3Vecs ...
  TVectorF& Mult3Vec(TVectorF& v) const;
  TVectorF& Rot3Vec(TVectorF& v) const;

  void OrtoNorm3();

  void Invert() { TMatrixF::InvertFast(); bAsOK = false; }

  Float_t operator()(Int_t rown, Int_t coln) const
  { return (fElements[rown*fNcols + coln]); }

  Float_t& operator()(Int_t rown, Int_t coln)
  { return (fElements[rown*fNcols + coln]); }

  ClassDef(ZTrans, 1)
};

ostream& operator<<(ostream& s, const ZVec3& v);
ostream& operator<<(ostream& s, const ZTrans& t);

#endif
