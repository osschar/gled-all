// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZTrans.h"
#include <Glasses/ZNode.h>

ClassImp(ZTrans)

// Matrix Prototype
static const TMatrixF ZTransMatrixProto(5,5);
ZTrans ZTrans::UnitTrans;

/**************************************************************************/

void ZTrans::_init()
{ 
  mA1 = mA2 = mA3 = 0;
  bAsOK = false;
}

ZTrans::ZTrans() : TMatrixF(TMatrixFBase::kUnit, ZTransMatrixProto) { _init(); }

ZTrans::ZTrans(const TMatrixF& m) : TMatrixF(m) { _init(); }

ZTrans::ZTrans(const ZTrans& z) : TMatrixF(z) { _init(); }

ZTrans::ZTrans(const ZNode* n) : TMatrixF(n->RefTrans()) { _init(); }

/**************************************************************************/

Int_t ZTrans::Set3Pos(Float_t x, Float_t y, Float_t z)
{
  TMatrixF& M = *this;
  M(1, 4) = x; M(2, 4) = y; M(3, 4) = z;
  return 1;
}

Int_t ZTrans::SetPos(const TVectorF& v)
{
  if(v.GetNoElements() != 5) return 1;
  TMatrixF& M = *this;
  for(Int_t i=0;i<5;i++)
    M(i, 4) = v(i);
  return 0;
}

Int_t ZTrans::SetPos(const ZTrans& t)
{
  if(t.GetNrows() != 5) return 1;
  TMatrixF& M = *this;
  for(Int_t i=0;i<5;i++)
    M(i, 4) = t(i,4);
  return 0;
}

TVectorF* ZTrans::GetZVector() const
{
  // *CHTD*; obsoleted by the next method
  TVectorF* v = new TVectorF(5);
  const TMatrixF& M = *this;
  for(Int_t i = 0; i<5; i++)
    (*v)(i) = M(i, 4);
  return v;
}

TVectorF* ZTrans::GetBaseV(UInt_t b) const
{
  // *CHTD*
  TVectorF* v = new TVectorF(5);
  const TMatrixF& M = *this;
  for(Int_t i = 0; i<5; i++)
    (*v)(i) = M(i, b);
  return v;
}

/**************************************************************************/

void ZTrans::SetRot(Int_t i, Int_t j, Float_t f)
{
  // Expects identity matrix
  TMatrixF& M = *this;
  M(i,i) = M(j,j) = TMath::Cos(f);
  Float_t s = TMath::Sin(f);
  M(i,j) = -s; M(j,i) = s;
  bAsOK = false;
}

void ZTrans::SetTrans(ZTrans& t) { *this = t;  bAsOK = false; }

void ZTrans::SetBaseV(UInt_t i, Float_t x, Float_t y, Float_t z)
{
  TMatrixF& M = *this;
  M(1,i) = x; M(2,i) = y; M(3,i) = z;
  bAsOK = false;
  // Should be stamped ... but not node ... safr
}

/**************************************************************************/
// Basic Transformations
/**************************************************************************/

Int_t ZTrans::MoveLF(Int_t ai, Float_t amount)
{
  TMatrixF& M = *this;
  for(Int_t i=0; i<=3; i++)
    M(i,4) += amount*M(i, ai);
  bAsOK = false;
  return 1;
}

Int_t ZTrans::RotateLF(Int_t i1, Int_t i2, Float_t amount)
{
  if(i1==i2) return 0;
  TMatrixF a(TMatrix::kUnit, ZT());
  a(i1,i1) = a(i2,i2) = TMath::Cos(amount);
  Float_t s = TMath::Sin(amount);
  a(i1,i2) = -s; a(i2,i1) = s;
  ZT() *= a;
  bAsOK = false;
  return 1;
}

Int_t ZTrans::Move(ZTrans* a, Int_t ai, Float_t amount)
{
  TMatrixF& M = *this;
  for(Int_t i=0; i<=3; i++)
    M(i,4) += amount * (*a)(i, ai);
  return 1;
}

Int_t ZTrans::Rotate(ZTrans* a, Int_t i1, Int_t i2, Float_t amount)
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
  inline void clamp_angle(Float_t& a) {
    while(a < -2*TMath::Pi()) a += 2*TMath::Pi();
    while(a >  2*TMath::Pi()) a -= 2*TMath::Pi();
  }
}

Int_t ZTrans::SetRotByAngles(Float_t a1, Float_t a2, Float_t a3)
{
  // Sets Rotation part as given by angles:
  // a1 around z, -a2 around y, a3 around x
  clamp_angle(a1); clamp_angle(a2); clamp_angle(a3);

  Float_t A, B, C, D, E, F;
  A = cos(a3); B = sin(a3);
  C = cos(a2); D = sin(a2); // should be -sin(a2) for positive direction
  E = cos(a1); F = sin(a1);
  Float_t AD = A*D, BD = B*D;

  TMatrixF& M = *this;
  M(1,1) = C*E; M(1,2) = -BD*E -A*F;  M(1,3) = -AD*E + B*F;
  M(2,1) = C*F; M(2,2) = -BD*F + A*E; M(2,3) = -AD*F - B*E;
  M(3,1) = D;   M(3,2) = B*C;         M(3,3) = A*C;

  mA1 = a1; mA2 = a2; mA3 = a3;
  bAsOK = true;
  return 1;
}

void ZTrans::Scale(Float_t sx, Float_t sy, Float_t sz)
{
  TMatrixF& M = *this;
  for(Int_t i=1; i<=3; ++i) {
    M(i,1) *= sx;
    M(i,2) *= sy;
    M(i,3) *= sz;
  }
}

ZVec3 ZTrans::Get3Rot() const
{
  const TMatrixF& M = *this;
  if(!bAsOK) {
    Float_t d = M(3,1);
    if(d>1) d=1; else if(d<-1) d=-1; // Fix numerical errors
    mA2 = TMath::ASin(d);
    Float_t C = TMath::Cos(mA2);
    if(TMath::Abs(C) > 8.7e-6) {
      mA1 = TMath::ATan2(M(2,1), M(1,1));      
      mA3 = TMath::ATan2(M(3,2), M(3,3));
    } else {
      mA1 = TMath::ATan2(M(2,1), M(2,2));
      mA3 = 0;
    }
    bAsOK = true;
  }
  return ZVec3(mA1, mA2, mA3);
}

/**************************************************************************/
// 3 Vec manipulation ... perhaps should have uniform funcs ... well ...
/**************************************************************************/

TVectorF& ZTrans::Mult3Vec(TVectorF& v) const
{
  // Multiplies v in-place w/ spatial part of mM ... takes Scale to be 1
  const TMatrixF& M = *this;
  Float_t buf[4];
  for(Int_t i=0; i<=2; i++) buf[i]=v(i);
  buf[3] = 1; // Scale

  for(Int_t i=0; i<=2; i++) {
    v(i) = 0;
    for(Int_t j=0; j<=3; j++)
      v(i) += M(i+1, j+1)*buf[j];
  }
  return v;
}

TVectorF& ZTrans::Rot3Vec(TVectorF& v) const
{
  // Multiplies v in-place w/ rotational-spatial part of mM ... Scale not used
  const TMatrixF& M = *this;
  Float_t buf[3];
  for(Int_t i=0; i<=2; i++) buf[i]=v(i);

  for(Int_t i=0; i<=2; i++) {
    v(i) = 0;
    for(Int_t j=0; j<=2; j++)
      v(i) += M(i+1, j+1)*buf[j];
  }
  return v;
}

/**************************************************************************/
// Pure matrix mogrification
/**************************************************************************/

void ZTrans::Transpose()
{
  TMatrixF& M = *this;
  Float_t b;
  for(Int_t i=0; i<5; i++) {
    for(Int_t j=i+1; j<5; j++) {
      b = M(i,j); M(i,j) = M(j,i); M(j,i) = b;
    }
  }
  bAsOK = false;
}

void ZTrans::Invert()
{
  TMatrixF& M = *this;
  Transpose();
  Double_t res;
  for(Int_t i=0; i<4; i++) {
    res = 0;
    for(Int_t j=0; j<4; j++)
      res -= M(i,j)*M(4,j);
    M(i,4) = res;
  }
  for(Int_t i=0; i<4; i++)
    M(4,i) = 0;
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
  for(Int_t i=0; i<5; i++)
    for(Int_t j=0; j<5; j++)
      s << t(i,j) << ((j==4) ? "\n" : "\t");
  return s;
}
