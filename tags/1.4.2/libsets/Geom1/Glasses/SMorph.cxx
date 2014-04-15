// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SMorph.h"
#include <Glasses/ZImage.h>
#include "SMorph.c7"

#include <Stones/TubeTvor.h>

#include <TMath.h>

ClassImp(SMorph);

void SMorph::_init(Float_t r)
{
  // Override settings from ZGlass
  bUseDispList = true;

  // Override settings from ZNode
  bUseScale = true;
  mSx = mSy = mSz = r;

  mTLevel = 2; mPLevel = 3;
  mTx = mCx = mRz = 0;

  mThetaMin = 0;   mThetaMax = 1;
  mPhiMean  = 0.5; mPhiRange = 1;
  bEquiSurf = false;

  mTexture = 0;
  mTexX0 = 0; mTexY0 =  1;
  mTexXC = 1; mTexYC = -1;
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

  if (bTextured)
  {
    float T[2];
    T[0] = mTexX0 + mTexXC * phi / (TMath::TwoPi());
    T[1] = mTexY0 + mTexYC * TMath::ACos(ct) / TMath::Pi();
    if (mTexYOff != 0) T[0] += Int_t(T[1])*mTexYOff;

    pTuber->NewVert(R, R, 0, T);
  }
  else
  {
    pTuber->NewVert(R, R);
  }
}

void SMorph::Triangulate()
{
  bTextured = (mTexture != 0);
  int tlevel = mTLevel + 1;
  pTuber->Init(0, tlevel, mPLevel, false, bTextured);

  float delta_t = TMath::Pi() * (mThetaMax - mThetaMin) / mTLevel;
  float t       = TMath::Pi() * mThetaMin;
  float last_ct = cos(t) + 2.0/mTLevel;
  for (int i=0; i<tlevel; i++)
  {
    float ct, st;
    if (bEquiSurf)
    {
      ct = last_ct - 2.0/mTLevel; if(ct < -1) ct = -1;
      st = sin(acos(ct));
    } else {
      ct = cos(t);
      st = sin(t);
      t += delta_t;
    }
    float phi  = TMath::TwoPi() * (mPhiMean - 0.5*mPhiRange);
    float step = TMath::TwoPi() * mPhiRange / (mPLevel);
    pTuber->NewRing(mPhiRange == 1 ? mPLevel : mPLevel - 1, true);
    for (int j=0; j<mPLevel; j++, phi+=step)
    {
      Messofy(ct, st, phi);
    }
    if (mPhiRange == 1)
      Messofy(ct, st, phi);
    last_ct = ct;
  }
}
