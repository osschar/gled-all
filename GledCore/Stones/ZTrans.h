// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZTrans_H
#define GledCore_ZTrans_H

#include <TVector3.h>

class ZNode;

/**************************************************************************/
// ZTrans -- 3D transformation in generalised coordinates
/**************************************************************************/

class ZTrans : public TObject
{
protected:
  Double32_t            M[16];

  mutable Float_t	mA1;   //!
  mutable Float_t	mA2;   //!
  mutable Float_t	mA3;   //!
  mutable Bool_t	bAsOK; //!

  Double_t norm3_column(Int_t col);
  Double_t orto3_column(Int_t col, Int_t ref);

public:
  ZTrans();
  ZTrans(const ZTrans& z);
  ZTrans(const ZNode* n);
  virtual ~ZTrans() {}

  // General operations

  void     UnitTrans();
  void     UnitRot();
  void     SetTrans(const ZTrans& t);
  ZTrans&  operator=(const ZTrans& t) { SetTrans(t); return *this; }
  void     SetupRotation(Int_t i, Int_t j, Double_t f);

  void     OrtoNorm3();
  Double_t Invert();

  void MultLeft(const ZTrans& t);
  void MultRight(const ZTrans& t);
  void operator*=(const ZTrans& t) { MultRight(t); }

  ZTrans operator*(const ZTrans& t);

  // Move & Rotate

  void MoveLF(Int_t ai, Double_t amount);
  void Move3LF(Double_t x, Double_t y, Double_t z);
  void RotateLF(Int_t i1, Int_t i2, Double_t amount);

  void MovePF(Int_t ai, Double_t amount);
  void Move3PF(Double_t x, Double_t y, Double_t z);
  void RotatePF(Int_t i1, Int_t i2, Double_t amount);

  void Move(const ZTrans& a, Int_t ai, Double_t amount);
  void Move3(const ZTrans& a, Double_t x, Double_t y, Double_t z);
  void Rotate(const ZTrans& a, Int_t i1, Int_t i2, Double_t amount);

  // Element access

  Double_t* Array() { return M; }      const Double_t* Array() const { return M; }
  Double_t* ArrX()  { return M; }      const Double_t* ArrX()  const { return M; }
  Double_t* ArrY()  { return M +  4; } const Double_t* ArrY()  const { return M +  4; }
  Double_t* ArrZ()  { return M +  8; } const Double_t* ArrZ()  const { return M +  8; }
  Double_t* ArrT()  { return M + 12; } const Double_t* ArrT()  const { return M + 12; }

  Double_t  operator[](Int_t i) const { return M[i]; }
  Double_t& operator[](Int_t i)       { return M[i]; }

  Double_t  CM(Int_t i, Int_t j) const { return M[4*j + i]; }
  Double_t& CM(Int_t i, Int_t j)       { return M[4*j + i]; }

  Double_t  operator()(Int_t i, Int_t j) const { return M[4*j + i - 5]; }
  Double_t& operator()(Int_t i, Int_t j)       { return M[4*j + i - 5]; }

  // Base-vector interface

  void SetBaseVec(Int_t b, Double_t x, Double_t y, Double_t z);
  void SetBaseVec(Int_t b, const TVector3& v);

  TVector3 GetBaseVec(Int_t b) const;
  void     GetBaseVec(Int_t b, TVector3& v) const;

  // Position interface

  void SetPos(Double_t x, Double_t y, Double_t z);
  void SetPos(Double_t* x);
  void SetPos(const ZTrans& t);

  void GetPos(Double_t& x, Double_t& y, Double_t& z) const;
  void GetPos(Double_t* x) const;
  void GetPos(TVector3& v) const;  
  TVector3 GetPos() const;

  // Cardan angle interface

  void SetRotByAngles(Float_t a1, Float_t a2, Float_t a3);
  void SetRotByAnyAngles(Float_t a1, Float_t a2, Float_t a3, const Text_t* pat);
  void GetRotAngles(Float_t* x) const;

  // Scaling

  void     Scale(Double_t sx, Double_t sy, Double_t sz);
  void     GetScale(Double_t& sx, Double_t& sy, Double_t& sz);
  void     Unscale(Double_t& sx, Double_t& sy, Double_t& sz);
  Double_t Unscale();


  // Operations on vectors

  void     MultiplyIP(TVector3& v, Double_t w=1);
  TVector3 Multiply(const TVector3& v, Double_t w=1);
  void     RotateIP(TVector3& v);
  TVector3 Rotate(const TVector3& v);


  virtual void Print(Option_t* option = "") const;

  ClassDef(ZTrans, 1) // Column-major 4x4 matrix for homogeneous coordinates.
};

ostream& operator<<(ostream& s, const ZTrans& t);

#endif
