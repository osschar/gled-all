// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// WSSeed
//
// Weaver Symbol Seed. Serves as container for WSPoints that make up
// the actual symbol. Provides triangulation service and stores
// triangulation data.

#include "WSSeed.h"
#include "WSSeed.c7"
#include "WSPoint.h"

#include <Tvor/TubeTvor.h>

#include <Gled/GThread.h>

#include <TSystem.h>

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

ClassImp(WSSeed)

/**************************************************************************/

void WSSeed::_init()
{
  mTLevel = 16; mPLevel = 16;

  mTexUOffset = mTexVOffset = 0;

  bRenormLen = false; mLength = 1;

  mLineW = 2; bFat = true;

  mTexture = 0;

  pTuber = 0;     bTextured = false;
  mTASleepMS = 50;  bTexAnimOn = false;
  mDtexU = -0.01; mDtexV = 0;

  bUseDispList = false;
}

WSSeed::~WSSeed() { delete pTuber; }

/**************************************************************************/

void WSSeed::SetTexUOffset(Float_t texu)
{
  mTexUOffset = texu;
  mStampReqTex = Stamp(FID());
}

void WSSeed::SetTexVOffset(Float_t texv)
{
  mTexVOffset = texv;
  mStampReqTex = Stamp(FID());
}

/**************************************************************************/

void WSSeed::Triangulate()
{
  // Should be called under ReadLock.

  list<WSPoint*> points; CopyByGlass<WSPoint*>(points);
  int size = points.size();
  if(!bFat || size < 2) {
    // !!!!! Could at least call Coffs !!!!!
    // Then don't need them in line rnr
    // and in TransAtTime
    return;
  }

  bTextured = (mTexture != 0);
  if(pTuber == 0) pTuber = new TubeTvor;
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
  ZTrans* lcfp;
  while((a=b++, b) != points.end()) {
    (*a)->Coff(*b);
    if(first) {
      first = false;
      lcfp = init_slide(*a);
      hTexU = mTexUOffset; hTexV = mTexVOffset;
    }
    Float_t delta = 1.0/mTLevel, max = 1 - delta/2;
    for(Float_t t=0; t<max; t+=delta) {
      ring(*lcfp, *a, t);
      hTexU += delta * (*a)->mStretch * len_fac;
      hTexV += delta * (*a)->mTwist;
    }
  }
  ring(*lcfp, *(--a), 1);
  // Store TexOffsets for re-texturing.
  hTexU = mTexUOffset; hTexV = mTexVOffset;
  delete lcfp;
}

void WSSeed::ReTexturize()
{
  // Should be called under ReadLock.

  Float_t du = mTexUOffset - hTexU;
  Float_t dv = mTexVOffset - hTexV;

  if(pTuber != 0 && pTuber->bTexP) {
    float* t = pTuber->mT;
    for(int i=0; i<pTuber->mNP; ++i, t+=2) {
      t[0] += du;
      t[1] += dv;
    }
  }
  hTexU = mTexUOffset;
  hTexV = mTexVOffset;

  mStampTexDone = mTimeStamp;
}

/**************************************************************************/

void WSSeed::TexAnimStart()
{
  if(bTexAnimOn) return;

  {
    GLensWriteHolder wlck(this);
    bTexAnimOn = true;
    Stamp(FID());
  }
  while(bTexAnimOn) {
    {
      GLensWriteHolder wlck(this);
      mTexUOffset = mTexUOffset + mDtexU;
      mTexVOffset = mTexVOffset + mDtexV;
      mStampReqTex = Stamp(FID());
    }
    GThread::TestCancel();
    gSystem->Sleep(mTASleepMS);
    GThread::TestCancel();
  }
}

void WSSeed::TexAnimStop()
{
  if(!bTexAnimOn) return;
  bTexAnimOn = false;
  Stamp(FID());
}

/**************************************************************************/

void WSSeed::TransAtTime(ZTrans& lcf, Float_t time, bool repeat_p)
{
  // Positions lcf to time.
  // If repeat_p is true, the time is clamped into the range.
  // 'Up' and 'right' axes are Gram-Schmidt orto-normalized wrt 'front'.

  static const string _eh("WSSeed::TransAtTime ");

  list<WSPoint*> points; CopyByGlass<WSPoint*>(points);
  int size = points.size();
  if(size < 2) {
    throw(_eh + "not enough points.");
  }

  // Here should call triangulate if mStamReqTex < mTimeStamp
  // The triangulation could measure lengths etc, store them in local data.
  // Then also don't need Coff call below.

  list<WSPoint*>::iterator a, b;
  Float_t len_fac = 1;
  Float_t len = 0;
  b = points.begin();
  while((a=b++, b) != points.end()) len += (*a)->mStretch;
  if(bRenormLen) {
    len_fac = mLength/len;
    len     = mLength;
  }

  if(time < 0 || time > len) {
    if(repeat_p) {
      time -= ((int)(time/len))*len;
      if(time < 0) time += len;
    } else {
      time = (time < 0) ? 0 : len;
    }
  }
  
  // printf("OK ... came up with time=%f\n", time);

  Float_t done = 0;
  b = points.begin();
  while((a=b++, b) != points.end()) {
    done += (*a)->mStretch * len_fac;
    if(done >= time) break;
  }
  Float_t t = 1  -  (done - time) / ((*a)->mStretch * len_fac);

  // printf("  limits '%s', '%s'; rel.time=%f\n",
  //   (*a)->GetName(), (*b)->GetName(), t);


  (*a)->Coff(*b);
  TMatrixFColumn hPnt(lcf,4), hAxe(lcf,1);

  Float_t t2 = t*t;
  Float_t t3 = t2*t;

  WSPoint* f = *a;
  for(Int_t i=1; i<=3; i++) {
    hPnt(i) = f->mCoffs(i, 0) + f->mCoffs(i, 1)*t +
      f->mCoffs(i, 2)*t2 + f->mCoffs(i, 3)*t3;
    hAxe(i) = f->mCoffs(i, 1) + 2*f->mCoffs(i, 2)*t +
      3*f->mCoffs(i, 3)*t2;
  }

  lcf.OrtoNorm3();
}

/**************************************************************************/
// Protected
/**************************************************************************/

WSPoint* WSSeed::get_first_point()
{
  GMutexHolder lmh(mListMutex);
  WSPoint* p = 0;
  lpZGlass_i i = mGlasses.begin();
  while(i != mGlasses.end() && (p = dynamic_cast<WSPoint*>(*i)) == 0) ++i;
  return p;
}

ZTrans* WSSeed::init_slide(WSPoint* f)
{
  if(f == 0) f = get_first_point();
  if(f == 0) return 0;
  return new ZTrans(f->RefTrans());
}

void WSSeed::ring(ZTrans& lcf, WSPoint* f, Float_t t)
{
  TMatrixFColumn hPnt(lcf,4), hAxe(lcf,1), hUp(lcf,2), hAw(lcf,3);

  Float_t t2 = t*t;
  Float_t t3 = t2*t;
  for(Int_t i=1; i<=3; i++) {
    hPnt(i) = f->mCoffs(i, 0) + f->mCoffs(i, 1)*t +
      f->mCoffs(i, 2)*t2 + f->mCoffs(i, 3)*t3;
    hAxe(i) = f->mCoffs(i, 1) + 2*f->mCoffs(i, 2)*t +
      3*f->mCoffs(i, 3)*t2;
  }
  Float_t w = f->mCoffs(4, 0) + f->mCoffs(4, 1)*t +
    f->mCoffs(4, 2)*t2 + f->mCoffs(4, 3)*t3;
  Float_t dwdt = TMath::ATan(f->mCoffs(4, 1) + 2*f->mCoffs(4, 2)*t +
			    3*f->mCoffs(4, 3)*t2);
  Float_t dwc = TMath::Cos(dwdt);
  Float_t dws = TMath::Sin(dwdt);

  lcf.OrtoNorm3();

  pTuber->NewRing(mPLevel, true);
  Float_t phi = 0, step = 2*TMath::Pi()/mPLevel;
  Float_t R[3], N[3], T[2];
  for(int j=0; j<mPLevel; j++, phi-=step) {
    Float_t cp = TMath::Cos(phi), sp = TMath::Sin(phi);
    for(Int_t i=1; i<=3; ++i) {
      R[i-1] = hPnt(i) + cp*w*hUp(i) + sp*w*hAw(i);
      N[i-1] = -dws*hAxe(i) + dwc*(cp*hUp(i) + sp*hAw(i));
    }
    // perhaps should invert normals for w<0
    T[0] = hTexU; T[1] = hTexV  - phi/TMath::TwoPi();
    pTuber->NewVert(R, N, 0, T);
  }
  { // last one
    phi = -2*TMath::Pi();
    Float_t cp = TMath::Cos(phi), sp = TMath::Sin(phi);
    for(Int_t i=1; i<=3; ++i) {
      R[i-1] = hPnt(i) + cp*w*hUp(i) + sp*w*hAw(i);
      N[i-1] = -dws*hAxe(i) + dwc*(cp*hUp(i) + sp*hAw(i));
    }
    // perhaps should invert normals for w<0
    T[0] = hTexU; T[1] = hTexV + 1;
    pTuber->NewVert(R, N, 0, T);
  }

}
