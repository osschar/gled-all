// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_HTrans_H
#define GledCore_HTrans_H

#include <TMath.h>

#ifndef __CINT__
namespace Opcode
{
  class Point;
  class Matrix4x4;
}
#endif

template<class TT> class HTrans;

//==============================================================================
// HPoint -- a simple 3D point
//==============================================================================

template<class TT>
class HPoint
{
public:
  TT x, y, z;

  HPoint() : x(0), y(0), z(0) {}
  HPoint(const Float_t* p)  : x(p[0]), y(p[1]), z(p[2]) {}
  HPoint(const Double_t* p) : x(p[0]), y(p[1]), z(p[2]) {}
  HPoint(TT _x, TT _y, TT _z) : x(_x), y(_y), z(_z) {}

  operator const TT*() const { return &x; }
  operator       TT*()       { return &x; }

  const TT* Arr() const { return &x; }
        TT* Arr()       { return &x; }

  TT  operator [] (Int_t idx) const { return (&x)[idx]; }
  TT& operator [] (Int_t idx)       { return (&x)[idx]; }

  void Set(TT _x, TT _y, TT _z) { x = _x; y = _y; z = _z; }
  void Set(const Float_t* p)    { x = p[0]; y = p[1]; z = p[2]; }
  void Set(const Double_t* p)   { x = p[0]; y = p[1]; z = p[2]; }
  void Zero()                   { x = y = z = 0; }

  HPoint& Neg() { x = -x; y = -y; z = -z; return *this; }

  HPoint& Add(TT _x, TT _y, TT _z) { x += _x; y += _y; z += _z; return *this; }

  HPoint& operator+=(const HPoint& a)   { x += a.x;  y += a.y;  z += a.z;  return *this; }
  HPoint& operator+=(const Float_t*  v) { x += v[0]; y += v[1]; z += v[2]; return *this; }
  HPoint& operator+=(const Double_t* v) { x += v[0]; y += v[1]; z += v[2]; return *this; }

  HPoint& operator-=(const HPoint& a)   { x -= a.x;  y -= a.y;  z -= a.z;  return *this; }
  HPoint& operator-=(const Float_t*  v) { x -= v[0]; y -= v[1]; z -= v[2]; return *this; }
  HPoint& operator-=(const Double_t* v) { x -= v[0]; y -= v[1]; z -= v[2]; return *this; }

  HPoint& operator*=(TT f) { x *= f; y *= f; z *= f; return *this; }

  TT Dot(const HPoint& v)   const { return x*v.x  + y*v.y  + z*v.z;  }
  TT Dot(const Float_t* v)  const { return x*v[0] + y*v[1] + z*v[2]; }
  TT Dot(const Double_t* v) const { return x*v[0] + y*v[1] + z*v[2]; }

  TT SquareMagnitude() const { return x*x + y*y + z*z; }
  TT Magnitude()       const { return TMath::Sqrt(SquareMagnitude()); }
  TT Mag2()            const { return x*x + y*y + z*z; }
  TT Mag()             const { return TMath::Sqrt(SquareMagnitude()); }

  TT   Perp2() const { return x*x + y*y; }
  TT   Perp()  const { return TMath::Sqrt(Perp2()); }

  TT   Phi()      const { return TMath::ATan2(y, x); }
  TT   Theta()    const { return TMath::ATan2(Perp(), z); }
  TT   CosTheta() const { TT m = Mag(); return m == 0 ? 1 : z / m; }
  TT   Eta()      const;

  TT Normalize(TT length=1);

  TT SquareDistance(const HPoint& v) const;
  TT Distance(const HPoint& v) const;

  HPoint& Cross(const HPoint& a, const HPoint& b);
  HPoint  Cross(const HPoint& b) const;

  HPoint  Orthogonal() const;
  void    OrthoNormBase(HPoint& a, HPoint& b) const;

  void Print() const;

  // No ClassDef
};

template<class TT>
ostream& operator<<(ostream& s, const HPoint<TT>& t);

//------------------------------------------------------------------------------

template<typename TT>
inline TT HPoint<TT>::SquareDistance(const HPoint& b) const
{
   return ((x - b.x) * (x - b.x) +
           (y - b.y) * (y - b.y) +
           (z - b.z) * (z - b.z));
}

template<typename TT>
inline TT HPoint<TT>::Distance(const HPoint& b) const
{
  return TMath::Sqrt(SquareDistance(b));
}

template<typename TT>
HPoint<TT> HPoint<TT>::Cross(const HPoint<TT>& b) const
{
  return HPoint<TT>(y * b.z - z * b.y,
		    z * b.x - x * b.z,
		    x * b.y - y * b.x);
}

template<typename TT>
HPoint<TT>& HPoint<TT>::Cross(const HPoint<TT>& a, const HPoint<TT>& b)
{
  x = a.y * b.z - a.z * b.y;
  y = a.z * b.x - a.x * b.z;
  z = a.x * b.y - a.y * b.x;
  return *this;
}

//------------------------------------------------------------------------------

template<typename TT>
inline HPoint<TT> operator+(const HPoint<TT>& a, const HPoint<TT>& b)
{
   HPoint<TT> r(a);
   return r += b;
}

template<typename TT>
inline HPoint<TT> operator-(const HPoint<TT>& a, const HPoint<TT>& b)
{
   HPoint<TT> r(a);
   return r -= b;
}

template<typename TT>
inline HPoint<TT> operator*(const HPoint<TT>& a, TT b)
{
   HPoint<TT> r(a);
   return r *= b;
}

template<typename TT>
inline HPoint<TT> operator*(TT b, const HPoint<TT>& a)
{
   HPoint<TT> r(a);
   return r *= b;
}

//------------------------------------------------------------------------------
// HPointF
//------------------------------------------------------------------------------

class HPointF : public HPoint<Float_t>
{
  typedef HPoint<Float_t> TP;

public:
  HPointF() : TP() {}
  HPointF(const Float_t* p)  : TP(p) {}
  HPointF(const Double_t* p) : TP(p) {}
  HPointF(Float_t _x, Float_t _y, Float_t _z) : TP(_x, _y, _z) {}
  template <typename OO>
  HPointF(const HPoint<OO>& v) : TP(v.x, v.y, v.z) {}

#ifndef __CINT__
  operator Opcode::Point* () { return  (Opcode::Point*) &x; }
  operator Opcode::Point& () { return *(Opcode::Point*) &x; }
  operator const Opcode::Point* () const { return  (Opcode::Point*) &x; }
  operator const Opcode::Point& () const { return *(Opcode::Point*) &x; }
#endif

  ClassDefNV(HPointF, 1); // Simple, streamable 3D point.
};

//------------------------------------------------------------------------------
// HPointD
//------------------------------------------------------------------------------

class HPointD : public HPoint<Double_t>
{
  typedef HPoint<Double_t> TP;

public:
  HPointD() : TP() {}
  HPointD(const Float_t* p)  : TP(p) {}
  HPointD(const Double_t* p) : TP(p) {}
  HPointD(Double_t _x, Double_t _y, Double_t _z) : TP(_x, _y, _z) {}
  template <typename OO>
  HPointD(const HPoint<OO>& v) : TP(v.x, v.y, v.z) {}

  ClassDefNV(HPointD, 1); // Simple, streamable 3D point.
};


//==============================================================================
// HTrans -- 3D transformation in generalised coordinates
//==============================================================================

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

  operator const TT*() const { return M; }
  operator       TT*()       { return M; }


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


  // General operations

  void UnitTrans();
  void UnitRot();
  void SetTrans(const HTrans& t);
  void SetFromArray(const Float_t*  arr);
  void SetFromArray(const Double_t* arr);
  void SetupRotation(Int_t i, Int_t j, TT f);

  TT   Norm3Column(Int_t col);
  TT   Orto3Column(Int_t col, Int_t ref);
  TT   OrtoNorm3Column(Int_t col, Int_t ref);
  void OrtoNorm3();
  void SetBaseVecViaCross(Int_t i);

  void Transpose();
  void TransposeRotationPart();
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


  // Base-vector interface

  void SetBaseVec(Int_t b, TT x, TT y, TT z)
  { TT* C = M + 4*--b; C[0] = x; C[1] = y; C[2] = z; }

  template <typename OO>
  void SetBaseVec(Int_t b, const HPoint<OO>& v)
  { SetBaseVec(b, v.x, v.y, v.z); }

  void SetBaseVec(Int_t b, Float_t* x)
  { TT* C = M + 4*--b; C[0] = x[0]; C[1] = x[1]; C[2] = x[2]; }

  void SetBaseVec(Int_t b, Double_t* x)
  { TT* C = M + 4*--b; C[0] = x[0]; C[1] = x[1]; C[2] = x[2]; }

  TT*       PtrBaseVec(Int_t b)       { return &M[4*(b-1)]; }
  const TT* PtrBaseVec(Int_t b) const { return &M[4*(b-1)]; }
  TT*       PtrBaseVecX()             { return &M[0]; }
  const TT* PtrBaseVecX()       const { return &M[0]; }
  TT*       PtrBaseVecY()             { return &M[4]; }
  const TT* PtrBaseVecY()       const { return &M[4]; }
  TT*       PtrBaseVecZ()             { return &M[8]; }
  const TT* PtrBaseVecZ()       const { return &M[8]; }

  HPoint<TT> GetBaseVec(Int_t b) const
  { const TT* C = M + 4*--b; return HPoint<TT>(C[0], C[1], C[2]); }

  void GetBaseVec(Int_t b, Float_t* x) const
  { const TT* C = M + 4*--b; x[0] = C[0], x[1] = C[1], x[2] = C[2]; }

  void GetBaseVec(Int_t b, Double_t* x) const
  { const TT* C = M + 4*--b; x[0] = C[0], x[1] = C[1], x[2] = C[2]; }


  // Position interface (can also use base-vec with idx 4)

  void SetPos(TT x, TT y, TT z) { M[12] = x; M[13] = y; M[14] = z; }
  template <typename OO>
  void SetPos(const HPoint<OO>& v) { SetPos(v.x,   v.y,   v.z);   }
  void SetPos(const Float_t* x)    { SetPos(x[0],  x[1],  x[2]);  }
  void SetPos(const Double_t* x)   { SetPos(x[0],  x[1],  x[2]);  }
  void SetPos(const HTrans& t)     { SetPos(t[12], t[13], t[14]); }

  void GetPos(TT& x, TT& y, TT& z) const { x = M[12]; y = M[13]; z = M[14];  }
  HPoint<TT> GetPos()              const { return HPoint<TT>(M[12], M[13], M[14]); }
  void GetPos(Float_t* x)          const { x[0] = M[12]; x[1] = M[13]; x[2] = M[14]; }
  void GetPos(Double_t* x)         const { x[0] = M[12]; x[1] = M[13]; x[2] = M[14]; }
  TT*  PtrPos()                          { return &M[12]; }


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

  void MultiplyIP(HPoint<TT>& v, TT w=1) const;
  void RotateIP(HPoint<TT>& v) const;

  HPoint<TT> Multiply(const HPoint<TT>& v, TT w=1) const;
  HPoint<TT> Rotate(const HPoint<TT>& v) const;

  void MultiplyVec3IP(TT* in, TT w) const;
  void MultiplyVec3(const TT* in, TT w, TT* out) const;
  void RotateVec3IP(TT* in) const;
  void RotateVec3(const TT* in, TT* out) const;
  void RotateBackVec3(const TT* in, TT* out) const;

  void Print() const;

  // No ClassDef
};


template<class TT>
ostream& operator<<(ostream& s, const HTrans<TT>& t);


//------------------------------------------------------------------------------
// HTransF
//------------------------------------------------------------------------------

class HTransF : public HTrans<Float_t>
{
  typedef HTrans<Float_t> TP;

public:
  HTransF()                          : TP()  {}
  HTransF(const HTransF& z)          : TP(z) {}
  HTransF(const HTrans<Double_t>& z) : HTrans<Float_t>(z.Array()) {}
  ~HTransF() {}

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

  ClassDefNV(HTransF, 1);
};


//------------------------------------------------------------------------------
// HTransD
//------------------------------------------------------------------------------

class HTransD : public HTrans<Double_t>
{
  typedef HTrans<Double_t> TP;

public:
  HTransD() : TP() {}
  HTransD(const HTransD& z)         : TP(z) {}
  HTransD(const HTrans<Float_t>& z) : TP(z.Array()) {}
  ~HTransD() {}

  ClassDefNV(HTransD, 1);
};

#endif
