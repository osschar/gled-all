// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
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
  bRenormLen = false; mLength = 1;
  mLineW = 2; bFat = true;
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

  bTextured = (mTexture != 0);
  pTuber->Init(0, mTLevel*(size - 1) + 1, mPLevel, false, bTextured);

  list<WSPoint*>::iterator a, b;
  Float_t len_fac = 1;
  if(bRenormLen) {
    Float_t len = 0;
    b = points.begin();
    while((a=b++, b) != points.end()) len += (*a)->mStretch;
    len_fac = mLength/len;
  }

  bool first = true;
  b = points.begin();
  while((a=b++, b) != points.end()) {
    (*a)->Coff(*b);
    if(first) {
      first = false;
      InitSlide(*a);
    }
    Float_t delta = 1.0/mTLevel, max = 1 - delta/2;
    for(Float_t t=0; t<max; t+=delta) {
      Ring(*a, t);
      hTexU += delta * (*a)->mTwist;
      hTexV += delta * (*a)->mStretch * len_fac;
    }
  }
  Ring(*(--a), 1);
}

void WSSeed::InitSlide(WSPoint* f)
{
  const ZTrans& t = f->RefTrans();
  for(Int_t i=0; i<5; ++i) {
    hUp(i) = t(i, 2u);
    hAw(i) = t(i, 3u);
  }
  hTexU = hTexV = 0;
}

namespace {
  double orto_norm(TVectorF& v, const TVectorF& x, int M) {
    double dp = 0;
    for(Int_t i=0; i<M; ++i) dp   += v(i)*x(i);
    for(Int_t i=0; i<M; ++i) v(i) -= x(i)*dp;
    double ni = 1/TMath::Sqrt(v.Norm2Sqr()); 
    for(Int_t i=0; i<M; ++i) v(i) *= ni;
    return dp;
  }
}

void WSSeed::Ring(WSPoint* f, Float_t t)
{
  Float_t t2 = t*t;
  Float_t t3 = t2*t;
  for(Int_t i=1; i<=3; i++) {
    hPnt(i) = f->mCoffs(i,0u) + f->mCoffs(i,1u)*t +
      f->mCoffs(i,2u)*t2 + f->mCoffs(i,3u)*t3;
    hAxe(i) = f->mCoffs(i,1u) + 2*f->mCoffs(i,2u)*t +
      3*f->mCoffs(i,3u)*t2;
  }
  Float_t w = f->mCoffs(4u,0u) + f->mCoffs(4u,1u)*t +
    f->mCoffs(4u,2u)*t2 + f->mCoffs(4u,3u)*t3;
  Float_t dwdt = TMath::ATan(f->mCoffs(4u,1u) + 2*f->mCoffs(4u,2u)*t +
			    3*f->mCoffs(4u,3u)*t2);
  Float_t dwc = TMath::Cos(dwdt);
  Float_t dws = TMath::Sin(dwdt);
  TVectorF oneAxe(hAxe);
  oneAxe *= 1/TMath::Sqrt(oneAxe.Norm2Sqr());
  orto_norm(hUp, oneAxe, 4);
  orto_norm(hAw, oneAxe, 4);
  orto_norm(hAw, hUp, 4);

  pTuber->NewRing(mPLevel, true);
  Float_t phi = 0, step = 2*TMath::Pi()/mPLevel;
  Float_t R[3], N[3], T[2];
  for(int j=0; j<mPLevel; j++, phi-=step) {
    Float_t cp = TMath::Cos(phi), sp = TMath::Sin(phi);
    for(Int_t i=1; i<=3; ++i) {
      R[i-1] = hPnt(i) + cp*w*hUp(i) + sp*w*hAw(i);
      N[i-1] = -dws*oneAxe(i) + dwc*(cp*hUp(i) + sp*hAw(i));
    }
    // perhaps should invert normals for w<0
    T[0] = hTexU - phi/TMath::TwoPi(); T[1] = hTexV;
    pTuber->NewVert(R, N, 0, T);
  }
  { // last one
    phi = -2*TMath::Pi();
    Float_t cp = TMath::Cos(phi), sp = TMath::Sin(phi);
    for(Int_t i=1; i<=3; ++i) {
      R[i-1] = hPnt(i) + cp*w*hUp(i) + sp*w*hAw(i);
      N[i-1] = -dws*oneAxe(i) + dwc*(cp*hUp(i) + sp*hAw(i));
    }
    // perhaps should invert normals for w<0
    T[0] = hTexU + 1; T[1] = hTexV;
    pTuber->NewVert(R, N, 0, T);
  }

}

#include "WSSeed.c7"
