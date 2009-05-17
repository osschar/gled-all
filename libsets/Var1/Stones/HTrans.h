// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_HTrans_H
#define Var1_HTrans_H

#ifndef __CINT__
namespace Opcode
{
  class Point;
  class Matrix4x4;
}
#endif

template<class TT> class HTrans;

/**************************************************************************/
// HPoint -- a simple 3D point
/**************************************************************************/

#include <TVector3.h>

template<class TT>
class HPoint
{
public:
  TT x, y, z;

  HPoint() : x(0), y(0), z(0) {}
  HPoint(Float_t* p)  : x(p[0]), y(p[1]), z(p[2]) {}
  HPoint(Double_t* p) : x(p[0]), y(p[1]), z(p[2]) {}
  HPoint(TT _x, TT _y, TT _z) : x(_x), y(_y), z(_z) {}
  virtual ~HPoint() {}

  void Set(TT _x, TT _y, TT _z) { x = _x; y = _y; z = _z; }
  void Set(Float_t* p)  { x = p[0]; y = p[1]; z = p[2]; }
  void Set(Double_t* p) { x = p[0]; y = p[1]; z = p[2]; }
  void Zero()           { x = y = z = 0; }

  void Neg() { x = -x; y = -y; z = -z; }

  HPoint& operator+=(const HPoint& a)   { x += a.x;  y += a.y;  z += a.z;  return *this; }
  HPoint& operator+=(const Float_t*  v) { x += v[0]; y += v[1]; z += v[2]; return *this; }
  HPoint& operator+=(const Double_t* v) { x += v[0]; y += v[1]; z += v[2]; return *this; }

  HPoint& operator*=(TT f) { x *= f; y *= f; z *= f; return *this; }

  operator const TT*() const { return &x; }
  operator       TT*()       { return &x; }

  ClassDef(HPoint, 1); // Simple, streamable 3D point.
};

// Axel.
#ifndef __APPLE__
template class HPoint<Float_t>;
template class HPoint<Double_t>;
#endif

class HPointF : public HPoint<Float_t>
{
public:
  HPointF()            : HPoint<Float_t>() {}
  HPointF(Float_t* p)  : HPoint<Float_t>(p) {}
  HPointF(Double_t* p) : HPoint<Float_t>(p) {}
  HPointF(Float_t _x, Float_t _y, Float_t _z) : HPoint<Float_t>(_x, _y, _z) {}
  virtual ~HPointF() {}

#ifndef __CINT__
  operator Opcode::Point* () { return  (Opcode::Point*) &x; }
  operator Opcode::Point& () { return *(Opcode::Point*) &x; }
  operator const Opcode::Point* () const { return  (Opcode::Point*) &x; }
  operator const Opcode::Point& () const { return *(Opcode::Point*) &x; }
#endif

  ClassDef(HPointF, 1); // Simple, streamable 3D point.
};

class HPointD : public HPoint<Double_t>
{
public:
  HPointD()            : HPoint<Double_t>() {}
  HPointD(Float_t* p)  : HPoint<Double_t>(p) {}
  HPointD(Double_t* p) : HPoint<Double_t>(p) {}
  HPointD(Double_t _x, Double_t _y, Double_t _z) : HPoint<Double_t>(_x, _y, _z) {}
  virtual ~HPointD() {}

  ClassDef(HPointD, 1); // Simple, streamable 3D point.
};

/******************************************************************************/
// HTrans -- 3D transformation in generalised coordinates
/******************************************************************************/

// export (not suppored)
template<class TT>
class HTrans
{
protected:
  TT       M[16];

public:
  HTrans();
  HTrans(const HTrans& z);
  HTrans(const Float_t*  x);
  HTrans(const Double_t* x);
  virtual ~HTrans() {}

  // General operations

  void     UnitTrans();
  void     UnitRot();
  void     SetTrans(const HTrans& t);
  HTrans&  operator=(const HTrans& t) { SetTrans(t); return *this; }
  void     SetupRotation(Int_t i, Int_t j, TT f);

  TT   Norm3Column(Int_t col);
  TT   Orto3Column(Int_t col, Int_t ref);
  TT   OrtoNorm3Column(Int_t col, Int_t ref);
  void OrtoNorm3();
  void SetBaseVecViaCross(Int_t i);

  void Transpose();
  TT   Invert();
  TT   InvertWithoutRow4();

  void MultLeft(const HTrans& t);
  void MultRight(const HTrans& t);
  void operator*=(const HTrans& t) { MultRight(t); }

  HTrans operator*(const HTrans& t);

  void MultLeft3x3(const TT* m);
  void MultRight3x3(const TT* m);

  void MultLeft3x3transposed(const TT* m);
  void MultRight3x3transposed(const TT* m);

  // Move & Rotate

  void MoveLF(Int_t ai, TT amount);
  void Move3LF(TT x, TT y, TT z);
  void RotateLF(Int_t i1, Int_t i2, TT amount);

  void MovePF(Int_t ai, TT amount);
  void Move3PF(TT x, TT y, TT z);
  void RotatePF(Int_t i1, Int_t i2, TT amount);

  void Move(const HTrans& a, Int_t ai, TT amount);
  void Move3(const HTrans& a, TT x, TT y, TT z);
  void Rotate(const HTrans& a, Int_t i1, Int_t i2, TT amount);

  // Element access

  TT* Array() { return M; }      const TT* Array() const { return M; }
  TT* ArrX()  { return M; }      const TT* ArrX()  const { return M; }
  TT* ArrY()  { return M +  4; } const TT* ArrY()  const { return M +  4; }
  TT* ArrZ()  { return M +  8; } const TT* ArrZ()  const { return M +  8; }
  TT* ArrT()  { return M + 12; } const TT* ArrT()  const { return M + 12; }

  TT  operator[](Int_t i) const { return M[i]; }
  TT& operator[](Int_t i)       { return M[i]; }

  TT  CM(Int_t i, Int_t j) const { return M[4*j + i]; }
  TT& CM(Int_t i, Int_t j)       { return M[4*j + i]; }

  TT  operator()(Int_t i, Int_t j) const { return M[4*j + i - 5]; }
  TT& operator()(Int_t i, Int_t j)       { return M[4*j + i - 5]; }

  // Base-vector interface

  void SetBaseVec(Int_t b, TT x, TT y, TT z)
  { TT* C = M + 4*--b; C[0] = x; C[1] = y; C[2] = z; }

  void SetBaseVec(Int_t b, TT* x)
  { TT* C = M + 4*--b; C[0] = x[0]; C[1] = x[1]; C[2] = x[2]; }

  void SetBaseVec(Int_t b, const TVector3& v)
  { TT* C = M + 4*--b; v.GetXYZ(C); }

  TT*       PtrBaseVec(Int_t b)       { return &M[4*(b-1)]; }
  const TT* PtrBaseVec(Int_t b) const { return &M[4*(b-1)]; }
  TT*       PtrBaseVecX()             { return &M[0]; }
  const TT* PtrBaseVecX()       const { return &M[0]; }
  TT*       PtrBaseVecY()             { return &M[4]; }
  const TT* PtrBaseVecY()       const { return &M[4]; }
  TT*       PtrBaseVecZ()             { return &M[8]; }
  const TT* PtrBaseVecZ()       const { return &M[8]; }

  void GetBaseVec(Int_t b, TT* x) const
  { const TT* C = M + 4*--b; x[0] = C[0], x[1] = C[1], x[2] = C[2]; }

  void GetBaseVec(Int_t b, TVector3& v) const
  { const TT* C = M + 4*--b; v.SetXYZ(C[0], C[1], C[2]); }

  TVector3 GetBaseVec(Int_t b) const
  { return TVector3(&M[4*--b]); }

  // Position interface (can also use base-vec with idx 4)

  void SetPos(TT x, TT y, TT z) { M[12] = x;     M[13] = y;     M[14] = z;     }
  void SetPos(TT* x)            { M[12] = x[0];  M[13] = x[1];  M[14] = x[2];  }
  void SetPos(const HTrans& t)  { M[12] = t[12]; M[13] = t[13]; M[14] = t[14]; }

  void GetPos(TT& x, TT& y, TT& z) const { x = M[12]; y = M[13]; z = M[14];  }
  void GetPos(TT* x)       const { x[0] = M[12]; x[1] = M[13]; x[2] = M[14]; }
  TT*  PtrPos()                  { return &M[12]; }
  void GetPos(TVector3& v) const { v.SetXYZ(M[12], M[13], M[14]); }
  TVector3 GetPos()        const { return TVector3(M[12], M[13], M[14]); }

  // Cardan angle interface

  void SetRotByAngles(Float_t a1, Float_t a2, Float_t a3);
  void SetRotByAnyAngles(Float_t a1, Float_t a2, Float_t a3, const Text_t* pat);
  void GetRotAngles(Float_t* x) const;

  // Scaling

  void Scale(TT sx, TT sy, TT sz);
  void GetScale(TT& sx, TT& sy, TT& sz) const;
  void Unscale(TT& sx, TT& sy, TT& sz);
  TT   Unscale();


  // Operations on vectors

  void     MultiplyIP(TVector3& v, TT w=1) const;
  TVector3 Multiply(const TVector3& v, TT w=1) const;
  void     RotateIP(TVector3& v) const;
  TVector3 Rotate(const TVector3& v) const;

  void     MultiplyVec3IP(TT* in, TT w) const;
  void     MultiplyVec3(const TT* in, TT w, TT* out) const;
  void     RotateVec3(const TT* in, TT* out) const;
  void     RotateBackVec3(const TT* in, TT* out) const;

  virtual void Print(Option_t* option = "") const;

  operator const TT*() const { return M; }
  operator       TT*()       { return M; }

  ClassDef(HTrans, 1);
};

// Include "less inline" members (export keyword not supported).

#include "HTrans.cpp"

template<class TT>
ostream& operator<<(ostream& s, const HTrans<TT>& t);

/**************************************************************************/
/**************************************************************************/

// The same thing ... Axel.
#ifndef __APPLE__
template class HTrans<Float_t>;
template class HTrans<Double_t>;
#endif

/**************************************************************************/
// Specializations
/**************************************************************************/

class HTransF : public HTrans<Float_t>
{
public:
  HTransF()                          : HTrans<Float_t>()  {}
  HTransF(const HTransF& z)          : HTrans<Float_t>(z) {}
  HTransF(const HTrans<Double_t>& z) : HTrans<Float_t>(z.Array()) {}
  virtual ~HTransF() {}

  using HTrans<Float_t>::SetBaseVec;
  using HTrans<Float_t>::GetBaseVec;

  void SetBaseVec(Int_t b, Double_t* x)
  { Float_t* C = M + 4*--b; C[0] = x[0]; C[1] = x[1]; C[2] = x[2]; }

  void GetBaseVec(Int_t b, Double_t* x) const
  { const Float_t* C = M + 4*--b; x[0] = C[0], x[1] = C[1], x[2] = C[2]; }

  using HTrans<Float_t>::SetPos;
  using HTrans<Float_t>::GetPos;

  void SetPos(Double_t* x) { M[12] = x[0]; M[13] = x[1]; M[14] = x[2]; }

  void GetPos(Double_t* x) const { x[0] = M[12]; x[1] = M[13]; x[2] = M[14]; }

#ifndef __CINT__
  Opcode::Point& ref_base_vec(Int_t i) { return * (Opcode::Point*) PtrBaseVec(i); }
  Opcode::Point& ref_base_vec_x()      { return * (Opcode::Point*) PtrBaseVecX(); }
  Opcode::Point& ref_base_vec_y()      { return * (Opcode::Point*) PtrBaseVecY(); }
  Opcode::Point& ref_base_vec_z()      { return * (Opcode::Point*) PtrBaseVecZ(); }
  Opcode::Point& ref_pos()             { return * (Opcode::Point*) PtrPos(); }

  operator Opcode::Matrix4x4* () { return  (Opcode::Matrix4x4*)M; }
  operator Opcode::Matrix4x4& () { return *(Opcode::Matrix4x4*)M; }
  operator const Opcode::Matrix4x4* () const { return  (Opcode::Matrix4x4*)M; }
  operator const Opcode::Matrix4x4& () const { return *(Opcode::Matrix4x4*)M; }
#endif

  ClassDef(HTransF, 1);
};


class HTransD : public HTrans<Double_t>
{
public:
  HTransD() : HTrans<Double_t>() {}
  HTransD(const HTransD& z)         : HTrans<Double_t>(z) {}
  HTransD(const HTrans<Float_t>& z) : HTrans<Double_t>(z.Array()) {}
  virtual ~HTransD() {}

  using HTrans<Double_t>::SetBaseVec;
  using HTrans<Double_t>::GetBaseVec;

  void SetBaseVec(Int_t b, Float_t* x)
  { Double_t* C = M + 4*--b; C[0] = x[0]; C[1] = x[1]; C[2] = x[2]; }

  void GetBaseVec(Int_t b, Float_t* x) const
  { const Double_t* C = M + 4*--b; x[0] = C[0], x[1] = C[1], x[2] = C[2]; }

  using HTrans<Double_t>::SetPos;
  using HTrans<Double_t>::GetPos;

  void SetPos(Float_t* x) { M[12] = x[0]; M[13] = x[1]; M[14] = x[2]; }
  void GetPos(Float_t* x) const { x[0 ] = M[12]; x[1] = M[13]; x[2] = M[14]; }

  ClassDef(HTransD, 1);
};

#endif
