// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SMorph.h"
#include <Tvor/TubeTvor.h>

#include <TMath.h>

ClassImp(SMorph)

void SMorph::_init(Float_t r)
{
  // Override settings from ZGlass
  bUseDispList = true;
  // Override settings from ZNode
  bUseScale = true;
  mSx = mSy = mSz = r;

  mTLevel = 2; mPLevel = 3;
  mTx = mCx = mRz = 0;

  bOpenTop = bOpenBot = bEquiSurf = false;

  mTexture = 0;
  mTexX0 = mTexY0 = 0;
  mTexXC = mTexYC = 1;
  mTexYOff = 0;

  pTuber = new TubeTvor;
  bTextured = false;
}

SMorph::~SMorph() { delete pTuber; }

/**************************************************************************/

void SMorph::Messofy(Float_t ct, Float_t st, Float_t phi)
{
  float R[3];
  Float_t twist = ct*mTx, conv = ct*mCx;
  Float_t x = ct, y = (1+conv)*st*cos(phi+twist), z = (1+conv)*st*sin(phi+twist);
  Float_t a = x*mRz, c=cos(a), s=sin(a);
  R[0] = x*c - y*s;
  R[1] = s*x + y*c;
  R[2] = z;

  float T[2];
  if(bTextured) {
    T[0] = mTexX0 + mTexXC * phi / (2*TMath::Pi());
    T[1] = mTexY0 + mTexYC * TMath::ACos(ct) / TMath::Pi();
    if(mTexYOff != 0) T[0] += Int_t(T[1])*mTexYOff;
  }

  pTuber->NewVert(R, R, 0, T);
}

void SMorph::Triangulate()
{
  bTextured = (mTexture != 0);
  int tlevel = mTLevel + 1; if(bOpenTop) --tlevel; if(bOpenBot) --tlevel;
  pTuber->Init(0, tlevel, mPLevel, false, bTextured);

  float last_ct = bOpenTop ? 1 : 1 + 2.0/mTLevel;
  float delta_t = TMath::Pi()/mTLevel;
  float t = bOpenTop ? delta_t : 0;
  for(int i=0; i<tlevel; i++) {
    float ct, st;
    if(bEquiSurf) {
      ct = last_ct - 2.0/mTLevel; if(ct < -1) ct = -1;
      st = sin(acos(ct));
    } else {
      ct = cos(t);
      st = sin(t);
      t += delta_t;
    }
    float phi = 0, step = 2*TMath::Pi()/mPLevel;
    pTuber->NewRing(mPLevel, true);
    for(int j=0; j<mPLevel; j++, phi+=step) {
      Messofy(ct, st, phi);
    }
    Messofy(ct, st, 2*TMath::Pi());
    last_ct = ct;
  }
}

#include "SMorph.c7"
