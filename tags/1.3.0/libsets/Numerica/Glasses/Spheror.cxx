// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Spheror.h"
#include <Glasses/ZQueen.h>

#include <TMath.h>


ClassImp(Spheror);

/**************************************************************************/

void Spheror::_init()
{
  mAmoeba = 0;
  mNVert=2; mNV=0;
  mSize=0.1; mScale=1;
}

/**************************************************************************/

TVectorF* Spheror::GimmeXYZ(const TVectorF& x)
{
  TVectorF* ret = new TVectorF(3*mNV); TVectorF& v = *ret;
  Int_t i=0, j=0;
  for(Int_t f=0; f<mNV; f++, i+=2, j+=3) {
    Float_t ct = TMath::Abs(TMath::Cos(x(i+1u)));
    Float_t cp = TMath::Cos(x(i)), sp = TMath::Sin(x(i));
    v(j) = ct*cp; v(j+1u) = ct*sp; v(j+2u) = TMath::Sin(x(i+1u));
  }
  return ret;
}

/**************************************************************************/

TVectorF* Spheror::InitialState(TRandom& rnd)
{
  mNV = mNVert;
  mState.ResizeTo(2*mNV);
  TVectorF* v = new TVectorF(2*mNV);
  for(Int_t i=0; i<2*mNV; i+=2) {
    (*v)(i)   = mState(i)    = 2*TMath::Pi()*rnd.Rndm();       // phi
    (*v)(i+1) = mState(i+1u) = TMath::ASin( 2*rnd.Rndm() - 1); // theta
  }
  return v;
}

TVectorF* Spheror::InitialPerturbations(TRandom& rnd)
{
  TVectorF* p = new TVectorF(2*mNV);
  for(Int_t i=0; i<2*mNV; i+=2) {
    (*p)(i) = TMath::Pi()/2 * ( rnd.Rndm() - 0.5 );
    (*p)(i+1) = TMath::Pi()/4 * ( rnd.Rndm() - 0.5 );
  }
  return p;
}

Float_t Spheror::Foon(const TVectorF& x)
{
  // Energy defed as sum of 1/r
  TVectorF* nkrumah = GimmeXYZ(x); TVectorF& v = *nkrumah;
  Float_t sum = 0;
  Int_t i=0;
  for(Int_t f=0; f<mNV-1; f++, i+=3) {
    Int_t j = i+3;
    for(Int_t g=f+1; g<mNV; g++, j+=3) {
#define SQR(a)	((a) * (a))
      Float_t drs = SQR(v(i)-v(j)) + SQR(v(i+1u)-v(j+1u)) + SQR(v(i+2u)-v(j+2u));
#undef SQR
      if(drs<1e-14) drs = 1e-14;
      sum += 1/TMath::Sqrt(drs);
    }
  }
  delete nkrumah;
  return sum;
}

void Spheror::SetState(const TVectorF& x)
{
  if(mState.GetNrows() != x.GetNrows()) {
    mState.ResizeTo(x);
    mNV = mState.GetNrows()/2;
  }
  mState = x;
  Stamp(FID());
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
  auto_ptr<TVectorF> v(InitialState(rnd));
  SetState(*v);
}

/**************************************************************************/

#include "Spheror.c7"
