// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZTrans_H
#define GledCore_ZTrans_H

// Includes
class ZNode;
class ZMark;
class ZVector;

#include <Stones/ZMatrix.h>
#include <TMath.h>

// Should rewrite this using ZVector (now that i have GetArray)
struct ZVec3 {
  Real_t v[3];
  ZVec3() { v[0] = v[1] = v[2] = 0; }
  ZVec3(Real_t x, Real_t y, Real_t z) { v[0]=x; v[1]=y; v[2]=z; }
  Real_t& operator()(Int_t i) { return v[i]; }
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

class ZTrans : public ZMatrix {

  mutable Real_t	mA1;   //!
  mutable Real_t	mA2;   //!
  mutable Real_t	mA3;   //!
  mutable Bool_t	bAsOK; //!

  void _init() { bAsOK = false; }

public:
  ZTrans();
  ZTrans(const ZMatrix& m);
  ZTrans(const ZTrans& z);
  ZTrans(const ZNode* n);
  virtual ~ZTrans() {}

  // more like define ... to use insted of (*this)
  ZTrans& ZT() { return *this; }

  Int_t Set3Pos(Real_t x, Real_t y, Real_t z);
  Int_t SetPos(const ZVector& v);
  Int_t SetPos(const ZTrans& t);
  ZVec3 Get3Pos() const { return ZVec3(At(1u,4u), At(2u,4u), At(3u,4u)); }

  ZVector* GetZVector() const;
  ZVector* GetBaseV(UInt_t b) const;

  void SetRot(UCIndex_t i, UCIndex_t j, Real_t f);
  void SetTrans(ZTrans& t);
  void SetBaseV(UInt_t i, Real_t x, Real_t y, Real_t z);

  // Space-Time Position
  // Move in LocalFrame; ai=1(fwd,bck), 2(up/down), 3(r/l)
  Int_t MoveLF(UCIndex_t ai, Real_t amount=1);
  // Rotate in LocalFrame; i1, i2 ... generator indices
  Int_t RotateLF(UCIndex_t i1, UCIndex_t i2, Real_t amount=0.02);
  // Now in some other frame ... 
  Int_t Move(ZTrans* a, UCIndex_t ai, Real_t amount=1);
  Int_t Rotate(ZTrans* a, UCIndex_t i1, UCIndex_t i2, Real_t amount=0.02);
  Int_t SetRotByAngles(Real_t a1, Real_t a2, Real_t a3);
  ZVec3 Get3Rot() const;

  // Stuff to do w/ 3Vecs ...
  ZVector& Mult3Vec(ZVector& v) const;
  ZVector& Rot3Vec(ZVector& v) const;

  void Transpose();
  void Invert();

  static ZTrans UnitTrans;

  ClassDef(ZTrans, 1)
};

ostream& operator<<(ostream& s, const ZVec3& v);
ostream& operator<<(ostream& s, const ZTrans& t);

#endif
