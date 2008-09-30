// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
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

public:
  ZTrans();
  ZTrans(const ZTrans& z);
  ZTrans(const ZNode* n);
  ZTrans(const Double_t arr[16]);
  ZTrans(const Float_t  arr[16]);
  virtual ~ZTrans() {}

  // General operations

  void     UnitTrans();
  void     UnitRot();
  void     SetTrans(const ZTrans& t);
  void     SetFromArray(const Double_t arr[16]);
  void     SetFromArray(const Float_t  arr[16]);
  ZTrans&  operator=(const ZTrans& t) { SetTrans(t); return *this; }
  void     SetupRotation(Int_t i, Int_t j, Double_t f);

  Double_t Norm3Column(Int_t col);
  Double_t Orto3Column(Int_t col, Int_t ref);
  Double_t OrtoNorm3Column(Int_t col, Int_t ref);
  void     OrtoNorm3();
  void     SetBaseVecViaCross(Int_t i);
  Double_t Invert();

  void MultLeft(const ZTrans& t);
  void MultRight(const ZTrans& t);
  void operator*=(const ZTrans& t) { MultRight(t); }

  void TransposeRotationPart();

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
  void SetBaseVec(Int_t b, const Double_t* x);
  void SetBaseVec(Int_t b, const Float_t*  x);

  TVector3 GetBaseVec(Int_t b) const;
  void     GetBaseVec(Int_t b, TVector3& v) const;
  void     GetBaseVec(Int_t b, Double_t* x) const;
  void     GetBaseVec(Int_t b, Float_t*  x) const;

  // Position interface

  void SetPos(Double_t x, Double_t y, Double_t z);
  void SetPos(const Double_t* x);
  void SetPos(const Float_t*  x);
  void SetPos(const ZTrans& t);

  void GetPos(Double_t& x, Double_t& y, Double_t& z) const;
  void GetPos(Double_t* x) const;
  void GetPos(Float_t*  x) const;
  void GetPos(TVector3& v) const;
  TVector3 GetPos() const;

  // Cardan angle interface

  void SetRotByAngles(Float_t a1, Float_t a2, Float_t a3);
  void SetRotByAnyAngles(Float_t a1, Float_t a2, Float_t a3, const Text_t* pat);
  void GetRotAngles(Float_t* x) const;

  // Scaling

  void     Scale(Double_t sx, Double_t sy, Double_t sz);
  void     GetScale(Double_t& sx, Double_t& sy, Double_t& sz) const;
  void     Unscale(Double_t& sx, Double_t& sy, Double_t& sz);
  Double_t Unscale();


  // Operations on vectors

  void     MultiplyIP(TVector3& v, Double_t w=1) const;
  TVector3 Multiply(const TVector3& v, Double_t w=1) const;
  void     RotateIP(TVector3& v) const;
  TVector3 Rotate(const TVector3& v) const;


  virtual void Print(Option_t* option = "") const;

  ClassDef(ZTrans, 1) // Column-major 4x4 matrix for homogeneous coordinates.
};

ostream& operator<<(ostream& s, const ZTrans& t);


/**************************************************************************/
// Base-vector interface
/**************************************************************************/

inline void ZTrans::SetBaseVec(Int_t b, Double_t x, Double_t y, Double_t z)
{
  Double_t* C = M + 4*--b;
  C[0] = x; C[1] = y; C[2] = z;
  bAsOK = false;
}

inline void ZTrans::SetBaseVec(Int_t b, const TVector3& v)
{
  Double_t* C = M + 4*--b;
  v.GetXYZ(C);
  bAsOK = false;
}

inline void ZTrans::SetBaseVec(Int_t b, const Double_t* x)
{
  Double_t* C = M + 4*--b;
  C[0] = x[0]; C[1] = x[1]; C[2] = x[2];
  bAsOK = false;
}

inline void ZTrans::SetBaseVec(Int_t b, const Float_t* x)
{
  Double_t* C = M + 4*--b;
  C[0] = x[0]; C[1] = x[1]; C[2] = x[2];
  bAsOK = false;
}

inline TVector3 ZTrans::GetBaseVec(Int_t b) const
{ return TVector3(&M[4*--b]); }

inline void ZTrans::GetBaseVec(Int_t b, TVector3& v) const
{
  const Double_t* C = M + 4*--b;
  v.SetXYZ(C[0], C[1], C[2]);
}

inline void ZTrans::GetBaseVec(Int_t b, Double_t* x) const
{
  const Double_t* C = M + 4*--b;
  x[0] = C[0], x[1] = C[1], x[2] = C[2];
}

inline void ZTrans::GetBaseVec(Int_t b, Float_t* x) const
{
  const Double_t* C = M + 4*--b;
  x[0] = C[0], x[1] = C[1], x[2] = C[2];
}

/**************************************************************************/
// Position interface
/**************************************************************************/

inline void ZTrans::SetPos(Double_t x, Double_t y, Double_t z)
{ M[12] = x; M[13] = y; M[14] = z; }

inline void ZTrans::SetPos(const Double_t* x)
{ M[12] = x[0]; M[13] = x[1]; M[14] = x[2]; }

inline void ZTrans::SetPos(const Float_t* x)
{ M[12] = x[0]; M[13] = x[1]; M[14] = x[2]; }

inline void ZTrans::SetPos(const ZTrans& t)
{
  const Double_t* T = t.M;
  M[12] = T[12]; M[13] = T[13]; M[14] = T[14];
}

inline void ZTrans::GetPos(Double_t& x, Double_t& y, Double_t& z) const
{ x = M[12]; y = M[13]; z = M[14]; }

inline void ZTrans::GetPos(Double_t* x) const
{ x[0] = M[12]; x[1] = M[13]; x[2] = M[14]; }

inline void ZTrans::GetPos(Float_t* x) const
{ x[0] = M[12]; x[1] = M[13]; x[2] = M[14]; }

inline void ZTrans::GetPos(TVector3& v) const
{ v.SetXYZ(M[12], M[13], M[14]); }

inline TVector3 ZTrans::GetPos() const
{ return TVector3(M[12], M[13], M[14]); }

#endif
