// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SMorph.h"
#include <Tvor/TubeTvor.h>

#include <TMath.h>

ClassImp(SMorph)

void SMorph::_init(Real_t r)
{
  mTLevel = 2; mPLevel = 3;
  mSx = mSy = mSz = r;
  mTx = mCx = mRz = 0;
  pTuber = new TubeTvor;
}

SMorph::~SMorph() { delete pTuber; }

/**************************************************************************/

void SMorph::Messofy(Real_t ct, Real_t st, Real_t phi)
{
  float R[3];
  Real_t twist = ct*mTx, conv = ct*mCx;
  Real_t x = ct, y = (1+conv)*st*cos(phi+twist), z = (1+conv)*st*sin(phi+twist);
  Real_t a = x*mRz, c=cos(a), s=sin(a);
  R[0] = x*c - y*s;
  R[1] = s*x + y*c;
  R[2] = z;
  pTuber->NewVert(R, R, 0);
}

void SMorph::Triangulate()
{
  pTuber->Init(false, 2, mTLevel-1, mPLevel);
  pTuber->NewRing(1, 1-bOpenTop);
  Messofy(1, 0, 0);
  float last_ct = 1;
  for(int i=1; i<mTLevel; i++) {
    float ct, st;
    if(bEquiSurf) {
      ct =  last_ct - 2.0/mTLevel;
      st = sin(acos(ct));
    } else {
      ct = cos(TMath::Pi()*i/mTLevel);
      st = sin(TMath::Pi()*i/mTLevel);
    }
    float phi = 0, step = 2*TMath::Pi()/mPLevel;
    pTuber->NewRing(mPLevel, true);
    for(int j=0; j<mPLevel; j++, phi+=step) {
      Messofy(ct, st, phi);
    }
    last_ct = ct;
  }
  pTuber->NewRing(1, 1-bOpenBot);
  Messofy(-1, 0, 0);
}

#include "SMorph.c7"
