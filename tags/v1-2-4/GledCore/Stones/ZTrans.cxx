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
  ZTrans& M = *this;
  M(1, 4) = x; M(2, 4) = y; M(3, 4) = z;
  return 1;
}

Int_t ZTrans::SetPos(const TVectorF& v)
{
  if(v.GetNoElements() != 5) return 1;
  ZTrans& M = *this;
  for(Int_t i=0;i<5;i++)
    M(i, 4) = v(i);
  return 0;
}

Int_t ZTrans::SetPos(const ZTrans& t)
{
  if(t.GetNrows() != 5) return 1;
  ZTrans& M = *this;
  for(Int_t i=0;i<5;i++)
    M(i, 4) = t(i,4);
  return 0;
}

TVectorF* ZTrans::GetZVector() const
{
  // *CHTD*; obsoleted by the next method
  TVectorF* v = new TVectorF(5);
  const ZTrans& M = *this;
  for(Int_t i = 0; i<5; i++)
    (*v)(i) = M(i, 4);
  return v;
}

TVectorF* ZTrans::GetBaseV(Int_t b) const
{
  // *CHTD*
  TVectorF* v = new TVectorF(5);
  const ZTrans& M = *this;
  for(Int_t i = 0; i<5; i++)
    (*v)(i) = M(i, b);
  return v;
}

TVector3 ZTrans::GetBaseVec3(Int_t b) const
{
  const ZTrans& M = *this;
  return TVector3(M(1,b), M(2,b), M(3,b));
}

/**************************************************************************/

void ZTrans::SetRot(Int_t i, Int_t j, Float_t f)
{
  // Expects identity matrix
  ZTrans& M = *this;
  M(i,i) = M(j,j) = TMath::Cos(f);
  Float_t s = TMath::Sin(f);
  M(i,j) = -s; M(j,i) = s;
  bAsOK = false;
}

void ZTrans::SetTrans(const ZTrans& t)
{
  TMatrixF::operator=(t);
  bAsOK = false;
}

void ZTrans::SetBaseV(Int_t i, Float_t x, Float_t y, Float_t z)
{
  ZTrans& M = *this;
  M(1,i) = x; M(2,i) = y; M(3,i) = z;
  bAsOK = false;
  // Should be stamped ... but not node ... safr
}

void ZTrans::SetBaseVec3(Int_t i, const TVector3& v)
{
  ZTrans& M = *this;
  M(1,i) = v.x(); M(2,i) = v.y(); M(3,i) = v.z();
  bAsOK = false;
  // Should be stamped ... but not node ... safr
}

/**************************************************************************/
// Basic Transformations
/**************************************************************************/

Int_t ZTrans::MoveLF(Int_t ai, Float_t amount)
{
  ZTrans& M = *this;
  for(Int_t i=0; i<=3; i++)
    M(i,4) += amount*M(i, ai);
  bAsOK = false;
  return 1;
}

Int_t ZTrans::Move3(Float_t x, Float_t y, Float_t z)
{
  ZTrans& M = *this;
  M(1,4) += x; M(2,4) += y; M(3,4) += z;
  bAsOK = false;
  return 1;
}

Int_t ZTrans::RotateLF(Int_t i1, Int_t i2, Float_t amount)
{
  if(i1==i2) return 0;
  ZTrans a;
  a(i1,i1) = a(i2,i2) = TMath::Cos(amount);
  Float_t s = TMath::Sin(amount);
  a(i1,i2) = -s; a(i2,i1) = s;
  *this *= a;
  bAsOK = false;
  return 1;
}

/**************************************************************************/

Int_t ZTrans::Move(ZTrans* a, Int_t ai, Float_t amount)
{
  ZTrans& M = *this;
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
  *a *= *this;
  *this = *a;
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

  ZTrans& M = *this;
  M(1,1) = C*E; M(1,2) = -BD*E -A*F;  M(1,3) = -AD*E + B*F;
  M(2,1) = C*F; M(2,2) = -BD*F + A*E; M(2,3) = -AD*F - B*E;
  M(3,1) = D;   M(3,2) = B*C;         M(3,3) = A*C;

  mA1 = a1; mA2 = a2; mA3 = a3;
  bAsOK = true;
  return 1;
}


ZVec3 ZTrans::Get3Rot() const
{
  const ZTrans& M = *this;
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

void ZTrans::Scale3(Float_t sx, Float_t sy, Float_t sz)
{
  Double_t s[4] = {0, sx, sy, sz};
  ZTrans& M = *this;
  for(int i=1;i<=3;++i)
    for(int j=1;j<=3;++j)
      M(j,i) *= s[i];
}

void ZTrans::GetScale3(Float_t& sx, Float_t& sy, Float_t& sz)
{
  ZTrans& M = *this;
  Double_t s[4] = {0,0,0,0};
  for(int i=1;i<=3;++i) {
    for(int j=1;j<=3;++j)
      s[i] += M(j, i) * M(j, i);
    s[i] = TMath::Sqrt(s[i]);
  }
  sx = s[1]; sy = s[2]; sz = s[3];
}

void ZTrans::Unscale3(Float_t& sx, Float_t& sy, Float_t& sz)
{
  ZTrans& M = *this;
  Double_t s[4] = {0,0,0,0};
  for(int i=1;i<=3;++i) {
    for(int j=1;j<=3;++j)
      s[i] += M(j, i) * M(j, i);
    s[i] = TMath::Sqrt(s[i]);
  }
  for(int i=1;i<=3;++i)
    for(int j=1;j<=3;++j)
      M(j,i) /= s[i];
  sx = s[1]; sy = s[2]; sz = s[3];
}


Float_t ZTrans::Unscale3()
{
  Float_t sx, sy, sz;
  Unscale3(sx, sy, sz);
  return (sx + sy + sz)/3;
}

/**************************************************************************/
// 3 Vec manipulation ... perhaps should have uniform funcs ... well ...
/**************************************************************************/

TVectorF& ZTrans::Mult3Vec(TVectorF& v) const
{
  // Multiplies v in-place w/ spatial part of mM ... takes Scale to be 1
  const ZTrans& M = *this;
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
  const ZTrans& M = *this;
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

Float_t ZTrans::norm3_column(Int_t col)
{
  ZTrans& M = *this;
  Double_t l = 0;
  for(Int_t i=1; i<=3; ++i) l += M(i,col)*M(i,col);
  l = TMath::Sqrt(l);
  for(Int_t i=1; i<=3; ++i) M(i,col) /= l;
  return l;
}

Float_t ZTrans::orto3_column(Int_t col, Int_t ref)
{
  ZTrans& M = *this;
  Double_t dp = 0;
  for(Int_t i=1; i<=3; ++i) dp       += M(i,col)*M(i,ref);
  for(Int_t i=1; i<=3; ++i) M(i,col) -= M(i,ref)*dp;
  return dp;
}

void ZTrans::OrtoNorm3()
{
  norm3_column(1);
  orto3_column(2,1); norm3_column(2);
  orto3_column(3,1); orto3_column(3,2); norm3_column(3);
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
