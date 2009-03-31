// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// HTrans
//
// HTrans is a 4x4 transformation matrix for homogeneous coordinates
// stored internaly in a column-major order to allow direct usage by
// GL. The element type is Double32_t as statically the floats would
// be precise enough but continuous operations on the matrix must
// retain precision of column vectors.
//
// Direct  element access (first two should be used with care):
// operator[i]    direct access to elements,   i:0->15
// CM(i,j)        element 4*j + i;           i,j:0->3    { CM ~ c-matrix }
// operator(i,j)  element 4*(j-1) + i - 1    i,j:1->4
//
// Column-vector access:
// USet Get/SetBaseVec(), Get/SetPos() and Arr[XYZT]() methods.
//
// For all methods taking the matrix indices:
// 1->X, 2->Y, 3->Z; 4->Position (if applicable). 0 reserved for time.
//
// Shorthands in method-names:
// LF ~ LocalFrame; PF ~ ParentFrame; IP ~ InPlace

#include "HTrans.h"
#include <Gled/GledTypes.h>
#include <TMath.h>

#define F00  0
#define F01  4
#define F02  8
#define F03 12

#define F10  1
#define F11  5
#define F12  9
#define F13 13

#define F20  2
#define F21  6
#define F22 10
#define F23 14

#define F30  3
#define F31  7
#define F32 11
#define F33 15

//ClassImp(HTrans);

/**************************************************************************/

template<class TT>
inline HTrans<TT>::HTrans() { UnitTrans(); }

template<class TT>
inline HTrans<TT>::HTrans(const HTrans<TT>& z) { SetTrans(z); }

template<class TT>
inline HTrans<TT>::HTrans(const Float_t* x)
{
  M[0]  = x[0];  M[1]  = x[1];  M[2]  = x[2];  M[3]  = x[3];
  M[4]  = x[4];  M[5]  = x[5];  M[6]  = x[6];  M[7]  = x[7];
  M[8]  = x[8];  M[9]  = x[9];  M[10] = x[10]; M[11] = x[11];
  M[12] = x[12]; M[13] = x[13]; M[14] = x[14]; M[15] = x[15];
}

template<class TT>
inline HTrans<TT>::HTrans(const Double_t* x)
{
  M[0]  = x[0];  M[1]  = x[1];  M[2]  = x[2];  M[3]  = x[3];
  M[4]  = x[4];  M[5]  = x[5];  M[6]  = x[6];  M[7]  = x[7];
  M[8]  = x[8];  M[9]  = x[9];  M[10] = x[10]; M[11] = x[11];
  M[12] = x[12]; M[13] = x[13]; M[14] = x[14]; M[15] = x[15];
}

/**************************************************************************/

template<class TT>
inline void HTrans<TT>::UnitTrans()
{
  M[0]  = 1; M[1]  = 0; M[2]  = 0; M[3]  = 0;
  M[4]  = 0; M[5]  = 1; M[6]  = 0; M[7]  = 0;
  M[8]  = 0; M[9]  = 0; M[10] = 1; M[11] = 0;
  M[12] = 0; M[13] = 0; M[14] = 0; M[15] = 1;
}

template<class TT>
inline void HTrans<TT>::UnitRot()
{
  M[0]  = 1; M[1]  = 0; M[2]  = 0;
  M[4]  = 0; M[5]  = 1; M[6]  = 0;
  M[8]  = 0; M[9]  = 0; M[10] = 1;
}

template<class TT>
inline void HTrans<TT>::SetTrans(const HTrans<TT>& t)
{
  const TT* const x = t.M;
  M[0]  = x[0];  M[1]  = x[1];  M[2]  = x[2];  M[3]  = x[3];
  M[4]  = x[4];  M[5]  = x[5];  M[6]  = x[6];  M[7]  = x[7];
  M[8]  = x[8];  M[9]  = x[9];  M[10] = x[10]; M[11] = x[11];
  M[12] = x[12]; M[13] = x[13]; M[14] = x[14]; M[15] = x[15];
}


template<class TT>
inline void HTrans<TT>::SetupRotation(Int_t i, Int_t j, TT f)
{
  // Setup the matrix as an elementary rotation.
  // Optimized versions of left/right multiplication with an elementary
  // rotation matrix are implemented in RotatePF/RotateLF.
  // Expects identity matrix.
  
  if(i == j) return;
  HTrans<TT>& M = *this;
  M(i,i) = M(j,j) = TMath::Cos(f);
  TT s = TMath::Sin(f);
  M(i,j) = -s; M(j,i) = s;
}

/**************************************************************************/
// OrtoNorm3 and Invert are near the bottom.
/**************************************************************************/

template<class TT>
inline void HTrans<TT>::MultLeft(const HTrans<TT>& t)
{
  TT  B[4];
  TT* C = M;
  for(int c=0; c<4; ++c, C+=4) {
    const TT* T = t.M;
    for(int r=0; r<4; ++r, ++T)
      B[r] = T[0]*C[0] + T[4]*C[1] + T[8]*C[2] + T[12]*C[3];
    C[0] = B[0]; C[1] = B[1]; C[2] = B[2]; C[3] = B[3];
  }
}

template<class TT>
inline void HTrans<TT>::MultRight(const HTrans<TT>& t)
{
  TT  B[4];
  TT* C = M;
  for(int r=0; r<4; ++r, ++C) {
    const TT* T = t.M;
    for(int c=0; c<4; ++c, T+=4)
      B[c] = C[0]*T[0] + C[4]*T[1] + C[8]*T[2] + C[12]*T[3];
    C[0] = B[0]; C[4] = B[1]; C[8] = B[2]; C[12] = B[3];
  }
}

template<class TT>
inline HTrans<TT> HTrans<TT>::operator*(const HTrans<TT>& t)
{
  HTrans<TT> b(*this);
  b.MultRight(t);
  return b;
}

template<class TT>
inline void HTrans<TT>::MultLeft3x3(const TT* m)
{
  // Multiply from left with a column-major 3x3 matrix.

  TT  B[3];
  TT* C = M;
  for(int c=0; c<3; ++c, C+=4) {
    const TT* T = m;
    for(int r=0; r<3; ++r, ++T)
      B[r] = T[0]*C[0] + T[3]*C[1] + T[6]*C[2];
    C[0] = B[0]; C[1] = B[1]; C[2] = B[2];
  }
}

template<class TT>
inline void HTrans<TT>::MultRight3x3(const TT* m)
{
  // Multiply from right with a column-major 3x3 matrix.

  TT  B[3];
  TT* C = M;
  for(int r=0; r<3; ++r, ++C) {
    const TT* T = m;
    for(int c=0; c<3; ++c, T+=3)
      B[c] = C[0]*T[0] + C[4]*T[1] + C[8]*T[2];
    C[0] = B[0]; C[4] = B[1]; C[8] = B[2];
  }
}

template<class TT>
inline void HTrans<TT>::MultLeft3x3transposed(const TT* m)
{
  // Multiply from left with the transpose of a column-major 3x3 matrix.
  // Alternatively - multiply from left with a row-major 3x3 matrix.

  TT  B[3];
  TT* C = M;
  for(int c=0; c<3; ++c, C+=4) {
    const TT* T = m;
    for(int r=0; r<3; ++r, T+=3)
      B[r] = T[0]*C[0] + T[1]*C[1] + T[2]*C[2];
    C[0] = B[0]; C[1] = B[1]; C[2] = B[2];
  }
}

template<class TT>
inline void HTrans<TT>::MultRight3x3transposed(const TT* m)
{
  // Multiply from right with the transpose of a column-major 3x3 matrix.
  // Alternatively - multiply from right with a row-major 3x3 matrix.

  TT  B[3];
  TT* C = M;
  for(int r=0; r<3; ++r, ++C) {
    const TT* T = m;
    for(int c=0; c<3; ++c, ++T)
      B[c] = C[0]*T[0] + C[4]*T[3] + C[8]*T[6];
    C[0] = B[0]; C[4] = B[1]; C[8] = B[2];
  }
}

/**************************************************************************/
// Move & Rotate
/**************************************************************************/

template<class TT>
inline void HTrans<TT>::MoveLF(Int_t ai, TT amount)
{
  const TT *C = M + 4*--ai;
  M[F03] += amount*C[0]; M[F13] += amount*C[1]; M[F23] += amount*C[2];
}

template<class TT>
inline void HTrans<TT>::Move3LF(TT x, TT y, TT z)
{
  M[F03] += x*M[0] + y*M[4] + z*M[8];
  M[F13] += x*M[1] + y*M[5] + z*M[9];
  M[F23] += x*M[2] + y*M[6] + z*M[10];
}

template<class TT>
inline void HTrans<TT>::RotateLF(Int_t i1, Int_t i2, TT amount)
{
  // Rotate in local frame. Does optimised version of MultRight.

  if(i1 == i2) return;
  // Algorithm: HTrans<TT> a; a.SetupRotation(i1, i2, amount); MultRight(a);
  // Optimized version:
  const TT cos = TMath::Cos(amount), sin = TMath::Sin(amount);
  TT  b1, b2;
  TT* C = M;
  --i1 <<= 2; --i2 <<= 2; // column major
  for(int r=0; r<4; ++r, ++C) {
    b1 = cos*C[i1] + sin*C[i2];
    b2 = cos*C[i2] - sin*C[i1];
    C[i1] = b1; C[i2] = b2;
  }
}

/**************************************************************************/

template<class TT>
inline void HTrans<TT>::MovePF(Int_t ai, TT amount)
{
  M[F03 + --ai] += amount;
}

template<class TT>
inline void HTrans<TT>::Move3PF(TT x, TT y, TT z)
{
  M[F03] += x;
  M[F13] += y;
  M[F23] += z;
}

template<class TT>
inline void HTrans<TT>::RotatePF(Int_t i1, Int_t i2, TT amount)
{
  // Rotate in parent frame. Does optimised version of MultLeft.

  if(i1 == i2) return;
  // Algorithm: HTrans<TT> a; a.SetupRotation(i1, i2, amount); MultLeft(a);

  // Optimized version:
  const TT cos = TMath::Cos(amount), sin = TMath::Sin(amount);
  TT  b1, b2;
  TT* C = M;
  --i1; --i2;
  for(int c=0; c<4; ++c, C+=4) {
    b1 = cos*C[i1] - sin*C[i2];
    b2 = cos*C[i2] + sin*C[i1];
    C[i1] = b1; C[i2] = b2;
  }
}

/**************************************************************************/

template<class TT>
inline void HTrans<TT>::Move(const HTrans<TT>& a, Int_t ai, TT amount)
{
  const TT* A = a.M + 4*--ai;
  M[F03] += amount*A[0];
  M[F13] += amount*A[1];
  M[F23] += amount*A[2];
}

template<class TT>
inline void HTrans<TT>::Move3(const HTrans<TT>& a, TT x, TT y, TT z)
{
  const TT* A = a.M;
  M[F03] += x*A[F00] + y*A[F01] + z*A[F02];
  M[F13] += x*A[F10] + y*A[F11] + z*A[F12];
  M[F23] += x*A[F20] + y*A[F21] + z*A[F22];
}

template<class TT>
inline void HTrans<TT>::Rotate(const HTrans<TT>& a, Int_t i1, Int_t i2, TT amount)
{
  if(i1 == i2) return;
  HTrans<TT> X(a);
  X.Invert();
  MultLeft(X);
  RotatePF(i1, i2, amount);
  MultLeft(a);
}

/**************************************************************************/
// Cardan angle interface
/**************************************************************************/

namespace {
  inline void clamp_angle(Float_t& a) {
    while(a < -TMath::TwoPi()) a += TMath::TwoPi();
    while(a >  TMath::TwoPi()) a -= TMath::TwoPi();
  }
}

template<class TT>
void HTrans<TT>::SetRotByAngles(Float_t a1, Float_t a2, Float_t a3)
{
  // Sets Rotation part as given by angles:
  // a1 around z, -a2 around y, a3 around x
  clamp_angle(a1); clamp_angle(a2); clamp_angle(a3);

  TT A, B, C, D, E, F;
  A = TMath::Cos(a3); B = TMath::Sin(a3);
  C = TMath::Cos(a2); D = TMath::Sin(a2); // should be -sin(a2) for positive direction
  E = TMath::Cos(a1); F = TMath::Sin(a1);
  TT AD = A*D, BD = B*D;

  M[F00] = C*E; M[F01] = -BD*E - A*F; M[F02] = -AD*E + B*F;
  M[F10] = C*F; M[F11] = -BD*F + A*E; M[F12] = -AD*F - B*E;
  M[F20] = D;   M[F21] = B*C;         M[F22] = A*C;
}

template<class TT>
void HTrans<TT>::SetRotByAnyAngles(Float_t a1, Float_t a2, Float_t a3,
			       const Text_t* pat)
{
  // Sets Rotation part as given by angles a1, a1, a3 and pattern pat.
  // Pattern consists of "XxYyZz" characters.
  // eg: x means rotate about x axis, X means rotate in negative direction
  // xYz -> R_x(a3) * R_y(-a2) * R_z(a1); (standard Gled representation)
  // Note that angles and pattern elements have inversed order!
  //
  // Implements Eulerian/Cardanian angles in a uniform way.

  int n = strspn(pat, "XxYyZz"); if(n > 3) n = 3;
  // Build Trans ... assign ...
  Float_t a[] = { a3, a2, a1 };
  UnitRot();
  for(int i=0; i<n; i++) {
    if(isupper(pat[i])) a[i] = -a[i];
    switch(pat[i]) {
    case 'x': case 'X': RotateLF(2, 3, a[i]); break;
    case 'y': case 'Y': RotateLF(3, 1, a[i]); break;
    case 'z': case 'Z': RotateLF(1, 2, a[i]); break;
    }
  }
}

template<class TT>
void HTrans<TT>::GetRotAngles(Float_t* x) const
{
  // Get Cardan rotation angles (pattern xYz above).
  // This only works if no scaling has been applied.

  TT d = M[F20];
  if(d>1) d=1; else if(d<-1) d=-1; // Fix numerical errors
  x[1] = TMath::ASin(d);
  TT C = TMath::Cos(x[1]);
  if(TMath::Abs(C) > 8.7e-6) {
    x[0] = TMath::ATan2(M[F10], M[F00]);      
    x[2] = TMath::ATan2(M[F21], M[F22]);
  } else {
    x[0] = TMath::ATan2(M[F10], M[F11]);
    x[2] = 0;
  }
}

/**************************************************************************/
// Scaling
/**************************************************************************/

template<class TT>
void HTrans<TT>::Scale(TT sx, TT sy, TT sz)
{
  M[F00] *= sx; M[F10] *= sx; M[F20] *= sx;
  M[F01] *= sy; M[F11] *= sy; M[F21] *= sy;
  M[F02] *= sz; M[F12] *= sz; M[F22] *= sz;
}

template<class TT>
void HTrans<TT>::GetScale(TT& sx, TT& sy, TT& sz) const
{
  sx = TMath::Sqrt( M[F00]*M[F00] + M[F10]*M[F10] + M[F20]*M[F20] );
  sy = TMath::Sqrt( M[F01]*M[F01] + M[F11]*M[F11] + M[F21]*M[F21] );
  sz = TMath::Sqrt( M[F02]*M[F02] + M[F12]*M[F12] + M[F22]*M[F22] );
}

template<class TT>
void HTrans<TT>::Unscale(TT& sx, TT& sy, TT& sz)
{
  GetScale(sx, sy, sz);
  M[F00] /= sx; M[F10] /= sx; M[F20] /= sx;
  M[F01] /= sy; M[F11] /= sy; M[F21] /= sy;
  M[F02] /= sz; M[F12] /= sz; M[F22] /= sz;
}

template<class TT>
TT HTrans<TT>::Unscale()
{
  TT sx, sy, sz;
  Unscale(sx, sy, sz);
  return (sx + sy + sz)/3;
}

/**************************************************************************/
// Operations on vectors
/**************************************************************************/

template<class TT>
inline void HTrans<TT>::MultiplyIP(TVector3& v, TT w) const
{
  v.SetXYZ(M[F00]*v.x() + M[F01]*v.y() + M[F02]*v.z() + M[F03]*w,
	   M[F10]*v.x() + M[F11]*v.y() + M[F12]*v.z() + M[F13]*w,
	   M[F20]*v.x() + M[F21]*v.y() + M[F22]*v.z() + M[F23]*w);
}

template<class TT>
inline TVector3 HTrans<TT>::Multiply(const TVector3& v, TT w) const
{
  return TVector3(M[F00]*v.x() + M[F01]*v.y() + M[F02]*v.z() + M[F03]*w,
		  M[F10]*v.x() + M[F11]*v.y() + M[F12]*v.z() + M[F13]*w,
		  M[F20]*v.x() + M[F21]*v.y() + M[F22]*v.z() + M[F23]*w);
}

template<class TT>
inline void HTrans<TT>::RotateIP(TVector3& v) const
{
  v.SetXYZ(M[F00]*v.x() + M[F01]*v.y() + M[F02]*v.z(),
	   M[F10]*v.x() + M[F11]*v.y() + M[F12]*v.z(),
	   M[F20]*v.x() + M[F21]*v.y() + M[F22]*v.z());
}

template<class TT>
inline TVector3 HTrans<TT>::Rotate(const TVector3& v) const
{
  return TVector3(M[F00]*v.x() + M[F01]*v.y() + M[F02]*v.z(),
		  M[F10]*v.x() + M[F11]*v.y() + M[F12]*v.z(),
		  M[F20]*v.x() + M[F21]*v.y() + M[F22]*v.z());
}

/**************************************************************************/

template<class TT>
inline void HTrans<TT>::MultiplyVec3IP(TT* in, TT w) const
{
  TT out[3];
  out[0] = M[F00]*in[0] + M[F01]*in[1] + M[F02]*in[2] + M[F03]*w;
  out[1] = M[F10]*in[0] + M[F11]*in[1] + M[F12]*in[2] + M[F13]*w;
  out[2] = M[F20]*in[0] + M[F21]*in[1] + M[F22]*in[2] + M[F23]*w;
  in[0] = out[0]; in[1] = out[1]; in[2] = out[2];
}

template<class TT>
inline void HTrans<TT>::MultiplyVec3(const TT* in, TT w, TT* out) const
{
  out[0] = M[F00]*in[0] + M[F01]*in[1] + M[F02]*in[2] + M[F03]*w;
  out[1] = M[F10]*in[0] + M[F11]*in[1] + M[F12]*in[2] + M[F13]*w;
  out[2] = M[F20]*in[0] + M[F21]*in[1] + M[F22]*in[2] + M[F23]*w;
}

template<class TT>
inline void HTrans<TT>::RotateVec3(const TT* in, TT* out) const
{
  out[0] = M[F00]*in[0] + M[F01]*in[1] + M[F02]*in[2];
  out[1] = M[F10]*in[0] + M[F11]*in[1] + M[F12]*in[2];
  out[2] = M[F20]*in[0] + M[F21]*in[1] + M[F22]*in[2];
}

template<class TT>
inline void HTrans<TT>::RotateBackVec3(const TT* in, TT* out) const
{
  // Rotate with transposed matrix. Note that this makes sense only if
  // the base vectors are orthonormal.

  out[0] = M[F00]*in[0] + M[F10]*in[1] + M[F20]*in[2];
  out[1] = M[F01]*in[0] + M[F11]*in[1] + M[F21]*in[2];
  out[2] = M[F02]*in[0] + M[F12]*in[1] + M[F22]*in[2];
}

/**************************************************************************/
// Normalization, ortogonalization
/**************************************************************************/

template<class TT>
inline TT HTrans<TT>::Norm3Column(Int_t col)
{
  TT* C = M + 4*--col;
  const TT l = 1.0/TMath::Sqrt(C[0]*C[0] + C[1]*C[1] + C[2]*C[2]);
  C[0] *= l; C[1] *= l; C[2] *= l;
  return l;
}

template<class TT>
inline TT HTrans<TT>::Orto3Column(Int_t col, Int_t ref)
{
  TT* C = M + 4*--col;
  TT* R = M + 4*--ref;
  const TT dp = C[0]*R[0] + C[1]*R[1] + C[2]*R[2];
  C[0] -= R[0]*dp; C[1] -= R[1]*dp; C[2] -= R[2]*dp;
  return dp;
}

template<class TT>
inline TT HTrans<TT>::OrtoNorm3Column(Int_t col, Int_t ref)
{
  // Ortogonalize col wrt ref then normalize col.
  // Returns dot-product.
  //
  // Assumption that both vectors are normalized and doing the calculation
  // in a single step resulted in slow degradation of norms.
  // The idea was ... since i know the projection on a future orthogonal vector
  // i also know the length after subtraction: len = sqrt(1 - dp^2).

  TT* C = M + 4*--col;
  TT* R = M + 4*--ref;
  const TT dp = C[0]*R[0] + C[1]*R[1] + C[2]*R[2];
  C[0] -= R[0]*dp; C[1] -= R[1]*dp; C[2] -= R[2]*dp;
  const TT l = 1.0/TMath::Sqrt(C[0]*C[0] + C[1]*C[1] + C[2]*C[2]);
  C[0] *= l; C[1] *= l; C[2] *= l;
  return dp;
}

template<class TT>
inline void HTrans<TT>::OrtoNorm3()
{
  Norm3Column(1);
  OrtoNorm3Column(2,1);
  M[F02] = M[F10]*M[F21] - M[F11]*M[F20];
  M[F12] = M[F20]*M[F01] - M[F21]*M[F00];
  M[F22] = M[F00]*M[F11] - M[F01]*M[F10];
  // cross-product faster.
  // Orto3Column(3,1); Orto3Column(3,2); Norm3Column(3);
}

template<class TT>
inline void HTrans<TT>::SetBaseVecViaCross(Int_t i)
{
  switch (i) {
  case 1:
    M[F00] = M[F11]*M[F22] - M[F12]*M[F21];
    M[F10] = M[F21]*M[F02] - M[F22]*M[F01];
    M[F20] = M[F01]*M[F12] - M[F02]*M[F11];
    break;
  case 2:
    M[F01] = M[F12]*M[F20] - M[F10]*M[F22];
    M[F11] = M[F22]*M[F00] - M[F20]*M[F02];
    M[F21] = M[F02]*M[F10] - M[F00]*M[F12];
    break;
  case 3:
    M[F02] = M[F10]*M[F21] - M[F11]*M[F20];
    M[F12] = M[F20]*M[F01] - M[F21]*M[F00];
    M[F22] = M[F00]*M[F11] - M[F01]*M[F10];
    break;
  default:
    break;
  }
}

/**************************************************************************/
// Transpose & Invert
/**************************************************************************/

template<class TT>
void HTrans<TT>::Transpose()
{
  TT x;
  x = M[F01]; M[F01] = M[F10]; M[F10] = x;
  x = M[F02]; M[F02] = M[F20]; M[F20] = x;
  x = M[F03]; M[F03] = M[F30]; M[F30] = x;
  x = M[F12]; M[F12] = M[F21]; M[F21] = x;
  x = M[F13]; M[F13] = M[F31]; M[F31] = x;
  x = M[F23]; M[F23] = M[F32]; M[F32] = x;  
}

template<class TT>
TT HTrans<TT>::Invert()
{
  // Copied from ROOT's TMatrixFCramerInv.

  static const Exc_t _eh("HTransF::Invert ");

  // Find all NECESSARY 2x2 dets:  (18 of them)
  const TT det2_12_01 = M[F10]*M[F21] - M[F11]*M[F20];
  const TT det2_12_02 = M[F10]*M[F22] - M[F12]*M[F20];
  const TT det2_12_03 = M[F10]*M[F23] - M[F13]*M[F20];
  const TT det2_12_13 = M[F11]*M[F23] - M[F13]*M[F21];
  const TT det2_12_23 = M[F12]*M[F23] - M[F13]*M[F22];
  const TT det2_12_12 = M[F11]*M[F22] - M[F12]*M[F21];
  const TT det2_13_01 = M[F10]*M[F31] - M[F11]*M[F30];
  const TT det2_13_02 = M[F10]*M[F32] - M[F12]*M[F30];
  const TT det2_13_03 = M[F10]*M[F33] - M[F13]*M[F30];
  const TT det2_13_12 = M[F11]*M[F32] - M[F12]*M[F31];
  const TT det2_13_13 = M[F11]*M[F33] - M[F13]*M[F31];
  const TT det2_13_23 = M[F12]*M[F33] - M[F13]*M[F32];
  const TT det2_23_01 = M[F20]*M[F31] - M[F21]*M[F30];
  const TT det2_23_02 = M[F20]*M[F32] - M[F22]*M[F30];
  const TT det2_23_03 = M[F20]*M[F33] - M[F23]*M[F30];
  const TT det2_23_12 = M[F21]*M[F32] - M[F22]*M[F31];
  const TT det2_23_13 = M[F21]*M[F33] - M[F23]*M[F31];
  const TT det2_23_23 = M[F22]*M[F33] - M[F23]*M[F32];

  // Find all NECESSARY 3x3 dets:   (16 of them)
  const TT det3_012_012 = M[F00]*det2_12_12 - M[F01]*det2_12_02 + M[F02]*det2_12_01;
  const TT det3_012_013 = M[F00]*det2_12_13 - M[F01]*det2_12_03 + M[F03]*det2_12_01;
  const TT det3_012_023 = M[F00]*det2_12_23 - M[F02]*det2_12_03 + M[F03]*det2_12_02;
  const TT det3_012_123 = M[F01]*det2_12_23 - M[F02]*det2_12_13 + M[F03]*det2_12_12;
  const TT det3_013_012 = M[F00]*det2_13_12 - M[F01]*det2_13_02 + M[F02]*det2_13_01;
  const TT det3_013_013 = M[F00]*det2_13_13 - M[F01]*det2_13_03 + M[F03]*det2_13_01;
  const TT det3_013_023 = M[F00]*det2_13_23 - M[F02]*det2_13_03 + M[F03]*det2_13_02;
  const TT det3_013_123 = M[F01]*det2_13_23 - M[F02]*det2_13_13 + M[F03]*det2_13_12;
  const TT det3_023_012 = M[F00]*det2_23_12 - M[F01]*det2_23_02 + M[F02]*det2_23_01;
  const TT det3_023_013 = M[F00]*det2_23_13 - M[F01]*det2_23_03 + M[F03]*det2_23_01;
  const TT det3_023_023 = M[F00]*det2_23_23 - M[F02]*det2_23_03 + M[F03]*det2_23_02;
  const TT det3_023_123 = M[F01]*det2_23_23 - M[F02]*det2_23_13 + M[F03]*det2_23_12;
  const TT det3_123_012 = M[F10]*det2_23_12 - M[F11]*det2_23_02 + M[F12]*det2_23_01;
  const TT det3_123_013 = M[F10]*det2_23_13 - M[F11]*det2_23_03 + M[F13]*det2_23_01;
  const TT det3_123_023 = M[F10]*det2_23_23 - M[F12]*det2_23_03 + M[F13]*det2_23_02;
  const TT det3_123_123 = M[F11]*det2_23_23 - M[F12]*det2_23_13 + M[F13]*det2_23_12;

  // Find the 4x4 det:
  const TT det = M[F00]*det3_123_123 - M[F01]*det3_123_023 + 
                 M[F02]*det3_123_013 - M[F03]*det3_123_012;

  if(det == 0) throw(_eh + "matrix is singular.");

  const TT oneOverDet = 1.0/det;
  const TT mn1OverDet = - oneOverDet;

  M[F00] = det3_123_123 * oneOverDet;
  M[F01] = det3_023_123 * mn1OverDet;
  M[F02] = det3_013_123 * oneOverDet;
  M[F03] = det3_012_123 * mn1OverDet;

  M[F10] = det3_123_023 * mn1OverDet;
  M[F11] = det3_023_023 * oneOverDet;
  M[F12] = det3_013_023 * mn1OverDet;
  M[F13] = det3_012_023 * oneOverDet;

  M[F20] = det3_123_013 * oneOverDet;
  M[F21] = det3_023_013 * mn1OverDet;
  M[F22] = det3_013_013 * oneOverDet;
  M[F23] = det3_012_013 * mn1OverDet;

  M[F30] = det3_123_012 * mn1OverDet;
  M[F31] = det3_023_012 * oneOverDet;
  M[F32] = det3_013_012 * mn1OverDet;
  M[F33] = det3_012_012 * oneOverDet;

  return det;
}

template<class TT>
TT HTrans<TT>::InvertWithoutRow4()
{
  // Copied from ROOT's TMatrixFCramerInv.
  // Optimised for 4th row = (0, 0, 0, 1).

  static const Exc_t _eh("HTrans::InvertWithoutRow4 ");

  // Find all NECESSARY 2x2 dets:  (18 of them, now 6)
  const TT det2_12_01 = M[F10]*M[F21] - M[F11]*M[F20];
  const TT det2_12_02 = M[F10]*M[F22] - M[F12]*M[F20];
  const TT det2_12_03 = M[F10]*M[F23] - M[F13]*M[F20];
  const TT det2_12_13 = M[F11]*M[F23] - M[F13]*M[F21];
  const TT det2_12_23 = M[F12]*M[F23] - M[F13]*M[F22];
  const TT det2_12_12 = M[F11]*M[F22] - M[F12]*M[F21];

  // Find all NECESSARY 3x3 dets:   (16 of them, now 12)
  const TT det3_012_013 = M[F00]*det2_12_13 - M[F01]*det2_12_03 + M[F03]*det2_12_01;
  const TT det3_012_023 = M[F00]*det2_12_23 - M[F02]*det2_12_03 + M[F03]*det2_12_02;
  const TT det3_012_123 = M[F01]*det2_12_23 - M[F02]*det2_12_13 + M[F03]*det2_12_12;

  const TT det3_013_013 = M[F00]*M[F11] - M[F01]*M[F10];
  const TT det3_013_023 = M[F00]*M[F12] - M[F02]*M[F10];
  const TT det3_013_123 = M[F01]*M[F12] - M[F02]*M[F11];
  const TT det3_023_013 = M[F00]*M[F21] - M[F01]*M[F20];
  const TT det3_023_023 = M[F00]*M[F22] - M[F02]*M[F20];
  const TT det3_023_123 = M[F01]*M[F22] - M[F02]*M[F21];
  const TT det3_123_013 = M[F10]*M[F21] - M[F11]*M[F20];
  const TT det3_123_023 = M[F10]*M[F22] - M[F12]*M[F20];
  const TT det3_123_123 = M[F11]*M[F22] - M[F12]*M[F21];

  // Find the 4x4 det:
  const TT det = M[F00]*det3_123_123 - M[F01]*det3_123_023 + M[F02]*det3_123_013;

  if(det == 0) throw(_eh + "matrix is singular.");

  const TT oneOverDet = 1.0/det;
  const TT mn1OverDet = - oneOverDet;

  M[F00] = det3_123_123 * oneOverDet;
  M[F01] = det3_023_123 * mn1OverDet;
  M[F02] = det3_013_123 * oneOverDet;
  M[F03] = det3_012_123 * mn1OverDet;

  M[F10] = det3_123_023 * mn1OverDet;
  M[F11] = det3_023_023 * oneOverDet;
  M[F12] = det3_013_023 * mn1OverDet;
  M[F13] = det3_012_023 * oneOverDet;

  M[F20] = det3_123_013 * oneOverDet;
  M[F21] = det3_023_013 * mn1OverDet;
  M[F22] = det3_013_013 * oneOverDet;
  M[F23] = det3_012_013 * mn1OverDet;

  // M[F30], M[F31], M[F32], M[F33] assumed (0, 0, 0, 1).

  return det;
}


/**************************************************************************/
/**************************************************************************/

#undef F00
#undef F01
#undef F02
#undef F03

#undef F10
#undef F11
#undef F12
#undef F13

#undef F20
#undef F21
#undef F22
#undef F23

#undef F30
#undef F31
#undef F32
#undef F33


/**************************************************************************/
/**************************************************************************/

template<class TT>
void HTrans<TT>::Print(Option_t* option) const
{
  const TT* C = M;
  for(Int_t i=0; i<4; ++i, ++C)
    printf("%8.3lf %8.3lf %8.3lf | %8.3lf\n", C[0], C[4], C[8], C[12]);
}

#include <iomanip>

template<class TT>
ostream& operator<<(ostream& s, const HTrans<TT>& t) {
  s.setf(ios::fixed, ios::floatfield); s.precision(3);
  for(Int_t i=1; i<=4; i++)
    for(Int_t j=1; j<=4; j++)
      s << t(i,j) << ((j==4) ? "\n" : "\t");
  return s;
}
