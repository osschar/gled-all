// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Spheror.h"
#include <Glasses/ZQueen.h>



ClassImp(Spheror)

/**************************************************************************/

void Spheror::_init()
{
  mAmoeba = 0;
  mNVert=2; mNV=0;
  mSize=0.1; mScale=1;
}

/**************************************************************************/

ZVector* Spheror::GimmeXYZ(const ZVector& x)
{
  ZVector* ret = new ZVector(3*mNV); ZVector& v = *ret;
  UCIndex_t i=0, j=0;
  for(UCIndex_t f=0; f<mNV; f++, i+=2, j+=3) {
    Real_t ct = TMath::Abs(TMath::Cos(x(i+1u)));
    Real_t cp = TMath::Cos(x(i)), sp = TMath::Sin(x(i));
    v(j) = ct*cp; v(j+1u) = ct*sp; v(j+2u) = TMath::Sin(x(i+1u));
  }
  return ret;
}

/**************************************************************************/

ZVector* Spheror::InitialState(TRandom& rnd)
{
  mNV = mNVert;
  mState.ResizeTo(2*mNV);
  ZVector* v = new ZVector(2*mNV);
  for(UCIndex_t i=0; i<2*mNV; i+=2) {
    (*v)(i)   = mState(i)    = 2*TMath::Pi()*rnd.Rndm();       // phi
    (*v)(i+1) = mState(i+1u) = TMath::ASin( 2*rnd.Rndm() - 1); // theta
  }
  return v;
}

ZVector* Spheror::InitialPerturbations(TRandom& rnd)
{
  ZVector* p = new ZVector(2*mNV);
  for(UCIndex_t i=0; i<2*mNV; i+=2) {
    (*p)(i) = TMath::Pi()/2 * ( rnd.Rndm() - 0.5 );
    (*p)(i+1) = TMath::Pi()/4 * ( rnd.Rndm() - 0.5 );
  }
  return p;
}

Real_t Spheror::Foon(const ZVector& x)
{
  // Energy defed as sum of 1/r
  ZVector* nkrumah = GimmeXYZ(x); ZVector& v = *nkrumah;
  Real_t sum = 0;
  UCIndex_t i=0;
  for(UCIndex_t f=0; f<mNV-1; f++, i+=3) {
    UCIndex_t j = i+3;
    for(UCIndex_t g=f+1; g<mNV; g++, j+=3) {
#define SQR(a)	((a) * (a))
      Real_t drs = SQR(v(i)-v(j)) + SQR(v(i+1u)-v(j+1u)) + SQR(v(i+2u)-v(j+2u));
#undef SQR
      if(drs<1e-14) drs = 1e-14;
      sum += 1/TMath::Sqrt(drs);
    }
  }
  delete nkrumah;
  return sum;
}

void Spheror::SetState(const ZVector& x)
{
  if(mState.GetNrows() != x.GetNrows()) {
    mState.ResizeTo(x);
    mNV = mState.GetNrows()/2;
  }
  mState = x;
  Stamp(LibID(), ClassID());
}

/**************************************************************************/

void Spheror::Install() {
  WarmAmoeba* wa = new WarmAmoeba(this, "SpheroAmoeba");
  mQueen->CheckIn(wa);
  Add(wa);
  wa->SetWA_Master(this);
  wa->SetStampInterval(1);
  SetAmoeba(wa);
}

void Spheror::SelfInit() {
  TRandom rnd(0);
  auto_ptr<ZVector> v(InitialState(rnd));
  SetState(*v);
}

/**************************************************************************/

#include "Spheror.c7"
