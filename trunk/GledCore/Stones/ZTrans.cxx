// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZTrans.h"
#include <Glasses/ZNode.h>

ClassImp(ZTrans)

// Matrix Prototype
static const ZMatrix ZTransMatrixProto(5,5);
ZTrans ZTrans::UnitTrans;

/**************************************************************************/

ZTrans::ZTrans() : ZMatrix(TMatrix::kUnit, ZTransMatrixProto) { _init(); }

ZTrans::ZTrans(const ZMatrix& m) : ZMatrix(m) { _init(); }

ZTrans::ZTrans(const ZTrans& z) : ZMatrix(z) { _init(); }

ZTrans::ZTrans(const ZNode* n) : ZMatrix(n->RefTrans()) { _init(); }

/**************************************************************************/

Int_t ZTrans::Set3Pos(Real_t x, Real_t y, Real_t z)
{
  At(1u,4u) = x; At(2u,4u) = y; At(3u,4u) = z; return 1;
}

Int_t ZTrans::SetPos(const ZVector& v)
{
  if(v.GetNoElements() != 5) return 1;
  for(UCIndex_t i=0;i<5;i++)
    At(i, 4u) = v(i);
  return 0;
}

Int_t ZTrans::SetPos(const ZTrans& t)
{
  if(t.GetNrows() != 5) return 1;
  for(UCIndex_t i=0;i<5;i++)
    At(i, 4u) = t(i,4u);
  return 0;
}

ZVector* ZTrans::GetZVector() const
{
  // *CHTD*; obsoleted by the next method
  ZVector* v = new ZVector(5);
  for(UCIndex_t i = 0; i<5; i++)
    (*v)(i) = At(i, 4u);
  return v;
}

ZVector* ZTrans::GetBaseV(UInt_t b) const
{
  // *CHTD*
  ZVector* v = new ZVector(5);
  for(UCIndex_t i = 0; i<5; i++)
    (*v)(i) = At(i, b);
  return v;
}

/**************************************************************************/

void ZTrans::SetRot(UCIndex_t i, UCIndex_t j, Real_t f)
{
  // Expects identity matrix
  At(i,i) = At(j,j) = TMath::Cos(f);
  Real_t s = TMath::Sin(f);
  At(i,j) = -s; At(j,i) = s;
  bAsOK = false;
}

void ZTrans::SetTrans(ZTrans& t) { *this = t;  bAsOK = false; }

void ZTrans::SetBaseV(UInt_t i, Real_t x, Real_t y, Real_t z)
{
  At(1u,i) = x; At(2u,i) = y; At(3u,i) = z;
  bAsOK = false;
  // Should be stamped ... but not node ... safr
}

/**************************************************************************/
// Basic Transformations
/**************************************************************************/

Int_t ZTrans::MoveLF(UCIndex_t ai, Real_t amount)
{
  for(UCIndex_t i=0; i<=3; i++)
    At(i,4u) += amount*At(i, ai);
  bAsOK = false;
  return 1;
}

Int_t ZTrans::RotateLF(UCIndex_t i1, UCIndex_t i2, Real_t amount)
{
  if(i1==i2) return 0;
  ZMatrix a(TMatrix::kUnit, ZT());
  a(i1,i1) = a(i2,i2) = TMath::Cos(amount);
  Real_t s = TMath::Sin(amount);
  a(i1,i2) = -s; a(i2,i1) = s;
  ZT() *= a;
  bAsOK = false;
  return 1;
}

Int_t ZTrans::Move(ZTrans* a, UCIndex_t ai, Real_t amount)
{
  for(UCIndex_t i=0; i<=3; i++)
    At(i,4u) += amount * a->At(i, ai);
  return 1;
}

Int_t ZTrans::Rotate(ZTrans* a, UCIndex_t i1, UCIndex_t i2, Real_t amount)
{
  // a is shitted
  if(i1==i2) return 0;
  ZTrans ai(*a); ai.Invert();
  ZTrans R; R.SetRot(i1, i2, amount); 
  *a *= R;
  *a *= ai;
  *a *= ZT();
  ZT() = *a;
  bAsOK = false;
  return 1;
}

namespace {
  inline void clamp_angle(Real_t& a) {
    while(a < -2*TMath::Pi()) a += 2*TMath::Pi();
    while(a >  2*TMath::Pi()) a -= 2*TMath::Pi();
  }
}

Int_t ZTrans::SetRotByAngles(Real_t a1, Real_t a2, Real_t a3)
{
  // Sets Rotation part as given by angles:
  // a1 around z, -a2 around y, a3 around x
  clamp_angle(a1); clamp_angle(a2); clamp_angle(a3);

  Real_t A, B, C, D, E, F;
  A = cos(a3); B = sin(a3);
  C = cos(a2); D = sin(a2); // should be -sin(a2) for positive direction
  E = cos(a1); F = sin(a1);
  Real_t AD = A*D, BD = B*D;

  At(1u,1u) = C*E; At(1u,2u) = -BD*E -A*F; At(1u,3u) = -AD*E + B*F;
  At(2u,1u) = C*F; At(2u,2u) = -BD*F + A*E; At(2u,3u) = -AD*F - B*E;
  At(3u,1u) = D; At(3u,2u) = B*C; At(3u,3u) = A*C;

  mA1 = a1; mA2 = a2; mA3 = a3;
  bAsOK = true;
  return 1;
}

ZVec3 ZTrans::Get3Rot() const
{
  if(!bAsOK) {
    Real_t d = At(3,1);
    if(d>1) d=1; else if(d<-1) d=-1; // Fix numerical errors
    mA2 = TMath::ASin(d);
    Real_t C = TMath::Cos(mA2);
    if(TMath::Abs(C) > 8.7e-6) {
      mA1 = TMath::ATan2(At(2,1), At(1,1));      
      mA3 = TMath::ATan2(At(3,2), At(3,3));
    } else {
      mA1 = TMath::ATan2(At(2,1), At(2,2));
      mA3 = 0;
    }
    bAsOK = true;
  }
  return ZVec3(mA1, mA2, mA3);
}

/**************************************************************************/
// 3 Vec manipulation ... perhaps should have uniform funcs ... well ...
/**************************************************************************/

ZVector& ZTrans::Mult3Vec(ZVector& v) const
{
  // Multiplies v in-place w/ spatial part of mM ... takes Scale to be 1
  Real_t buf[4];
  for(UCIndex_t i=0; i<=2; i++) buf[i]=v(i);
  buf[3] = 1; // Scale

  for(UCIndex_t i=0; i<=2; i++) {
    v(i) = 0;
    for(UCIndex_t j=0; j<=3; j++)
      v(i) += At(i+1u, j+1u)*buf[j];
  }
  return v;
}

ZVector& ZTrans::Rot3Vec(ZVector& v) const
{
  // Multiplies v in-place w/ rotational-spatial part of mM ... Scale not used
  Real_t buf[3];
  for(UCIndex_t i=0; i<=2; i++) buf[i]=v(i);

  for(UCIndex_t i=0; i<=2; i++) {
    v(i) = 0;
    for(UCIndex_t j=0; j<=2; j++)
      v(i) += At(i+1u, j+1u)*buf[j];
  }
  return v;
}

/**************************************************************************/
// Pure matrix mogrification
/**************************************************************************/

void ZTrans::Transpose()
{
  Real_t b;
  for(UCIndex_t i=0; i<5; i++) {
    for(UCIndex_t j=i+1; j<5; j++) {
      b = At(i,j); At(i,j) = At(j,i); At(j,i) = b;
    }
  }
  bAsOK = false;
}

void ZTrans::Invert()
{
  Transpose();
  Double_t res;
  for(UCIndex_t i=0; i<4; i++) {
    res = 0;
    for(UCIndex_t j=0; j<4; j++)
      res -= At(i,j)*At(4u,j);
    At(i,4u) = res;
  }
  for(UCIndex_t i=0; i<4; i++)
    At(4u,i) = 0;
  bAsOK = false;
}

/**************************************************************************/

#include <iomanip>

ostream& operator<<(ostream& s, const ZVec3& v) {
  s.setf(ios::fixed, ios::floatfield); s.precision(3);
  return s << v.v[0] << " " << v.v[1] << " " << v.v[2];
}

ostream& operator<<(ostream& s, const ZTrans& t) {
  s.setf(ios::fixed, ios::floatfield); s.precision(3);
  for(UCIndex_t i=0; i<5; i++)
    for(UCIndex_t j=0; j<5; j++)
      s << t(i,j) << ((j==4) ? "\n" : "\t");
  return s;
}
