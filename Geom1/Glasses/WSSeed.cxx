// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// WSSeed
//
// Weaver Symbol Seed. Serves as container for WSPoints that make up
// the actual symbol. Provides triangulation service and stores
// triangulation data.

#include "WSSeed.h"
#include "WSPoint.h"

#include <Tvor/TubeTvor.h>

ClassImp(WSSeed)

/**************************************************************************/

void WSSeed::_init()
{
  mTLevel = 10; mPLevel = 10;
  mLineW = 3; bFat = true;
  pTuber = new TubeTvor;
}

WSSeed::~WSSeed() { delete pTuber; }

/**************************************************************************/

void WSSeed::Triangulate()
{
  list<WSPoint*> points; CopyByGlass<WSPoint*>(points);
  int size = points.size();
  if(!bFat || size < 2) {
    return;
  }

  pTuber->Init(0, mTLevel*(size - 1) + 1, mPLevel);

  list<WSPoint*>::iterator a, b = points.begin();
  bool first = true;
  while((a=b++, b) != points.end()) {
    (*a)->Coff(*b);
    if(first) {
      first = false;
      InitSlide(*a);
    }
    Real_t delta = 1.0/mTLevel, max = 1 - delta/2;
    for(Real_t t=0; t<max; t+=delta) {
      Ring(*a, t);
    }
  }
  Ring(*(--a), 1);
}

void WSSeed::InitSlide(WSPoint* f)
{
  const ZTrans& t = f->RefTrans();
  for(UCIndex_t i=0; i<5; ++i) {
    hUp(i) = t(i, 2u);
    hAw(i) = t(i, 3u);
  }
}

void WSSeed::Ring(WSPoint* f, Real_t t)
{
  Real_t t2 = t*t;
  Real_t t3 = t2*t;
  for(UCIndex_t i=1; i<=3; i++) {
    hPnt(i) = f->mCoffs(i,0u) + f->mCoffs(i,1u)*t +
      f->mCoffs(i,2u)*t2 + f->mCoffs(i,3u)*t3;
    hAxe(i) = f->mCoffs(i,1u) + 2*f->mCoffs(i,2u)*t +
      3*f->mCoffs(i,3u)*t2;
  }
  Real_t w = f->mCoffs(4u,0u) + f->mCoffs(4u,1u)*t +
    f->mCoffs(4u,2u)*t2 + f->mCoffs(4u,3u)*t3;
  Real_t dwdt = TMath::ATan(f->mCoffs(4u,1u) + 2*f->mCoffs(4u,2u)*t +
			    3*f->mCoffs(4u,3u)*t2);
  Real_t dwc = TMath::Cos(dwdt);
  Real_t dws = TMath::Sin(dwdt);
  ZVector oneAxe(hAxe); oneAxe.Norm(4);
  hUp.OrtoNorm(oneAxe, 4);
  hAw.OrtoNorm(oneAxe, 4); hAw.OrtoNorm(hUp, 4);

  pTuber->NewRing(mPLevel, true);
  Real_t phi = 0, step = 2*TMath::Pi()/mPLevel;
  Real_t R[3], N[3];
  for(int j=0; j<mPLevel; j++, phi-=step) {
    Real_t cp = TMath::Cos(phi), sp = TMath::Sin(phi);
    for(UCIndex_t i=1; i<=3; ++i) {
      R[i-1] = hPnt(i) + cp*w*hUp(i) + sp*w*hAw(i);
      N[i-1] = -dws*oneAxe(i) + dwc*(cp*hUp(i) + sp*hAw(i));
    }
    // perhaps should invert normals for w<0
    pTuber->NewVert(R, N, 0);
  }
  { // last one
    phi = -2*TMath::Pi();
    Real_t cp = TMath::Cos(phi), sp = TMath::Sin(phi);
    for(UCIndex_t i=1; i<=3; ++i) {
      R[i-1] = hPnt(i) + cp*w*hUp(i) + sp*w*hAw(i);
      N[i-1] = -dws*oneAxe(i) + dwc*(cp*hUp(i) + sp*hAw(i));
    }
    // perhaps should invert normals for w<0
    pTuber->NewVert(R, N, 0);
  }

}

#include "WSSeed.c7"
