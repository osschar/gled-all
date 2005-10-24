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


//--- tmp ---
#include <Glasses/ZQueen.h>
#include <Glasses/Sphere.h>
#include <TRandom.h>

ClassImp(WSSeed)

/**************************************************************************/

void WSSeed::_init()
{
  // Override settings from ZGlass
  bUseDispList = true;

  mTLevel = 16; mPLevel = 16;

  mTexUOffset = mTexVOffset = 0;

  bRenormLen = false; mLength = 1;

  mLineW = 2; bFat = true;

  mTexture = 0;

  pTuber = 0;     bTextured = false;
  mTASleepMS = 50;  bTexAnimOn = false;
  mDtexU = -0.01; mDtexV = 0;
}

WSSeed::~WSSeed() { delete pTuber; }

/**************************************************************************/

void WSSeed::Triangulate()
{
  // Should be called under ReadLock.

  list<WSPoint*> points; CopyListByGlass<WSPoint>(points);
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
/**************************************************************************/

Float_t WSSeed::MeasureLength()
{
  GMutexHolder lstlck(mListMutex);

  Float_t len = 0;
  Stepper<WSPoint> s(this);
  WSPoint* a = *s;
  while(s.step()) {
    len += a->mStretch;
    a = *s;
  }
  return len;
}

void WSSeed::MeasureAndSetLength()
{
  mLength = MeasureLength();
  mStampReqTex = Stamp(FID());
}

/**************************************************************************/
/**************************************************************************/

void WSSeed::TransAtTime(ZTrans& lcf, Double_t time, Bool_t repeat_p, Bool_t reinit_trans_p)
{
  // Positions lcf to time.
  // If repeat_p is true, the time is clamped into the range.
  // 'Up' and 'right' axes are Gram-Schmidt orto-normalized wrt 'front'.

  static const Exc_t _eh("WSSeed::TransAtTime ");

  list<WSPoint*> points; CopyListByGlass<WSPoint>(points);
  int size = points.size();
  if(size < 2) {
    throw(_eh + "not enough points.");
  }

  // Here should call triangulate if mStamReqTex < mTimeStamp
  // The triangulation could measure lengths etc, store them in local data.
  // Then also don't need Coff call below.

  list<WSPoint*>::iterator a, b;
  Double_t len_fac = 1;
  Double_t len = 0;
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

  Double_t done = 0;
  b = points.begin();
  while((a=b++, b) != points.end()) {
    done += (*a)->mStretch * len_fac;
    if(done >= time) break;
  }
  Double_t t = 1  -  (done - time) / ((*a)->mStretch * len_fac);

  // printf("  limits '%s', '%s'; rel.time=%f\n",
  //   (*a)->GetName(), (*b)->GetName(), t);

  if(reinit_trans_p)
    lcf = (*a)->RefTrans();

  (*a)->Coff(*b);

  const Double_t t2 = t*t, t3 = t2*t;
  Double_t* Pnt = lcf.ArrT();
  Double_t* Axe = lcf.ArrX();

  for(Int_t i=0; i<3; i++) {
    const TMatrixDRow R( (*a)->mCoffs[i] );
    Pnt[i] = R[0] + R[1]*t + R[2]*t2 + R[3]*t3;
    Axe[i] = R[1] + 2*R[2]*t + 3*R[3]*t2;
  }

  lcf.OrtoNorm3();
}

/**************************************************************************/
// Protected
/**************************************************************************/

WSPoint* WSSeed::get_first_point()
{
  GMutexHolder lmh(mListMutex);
  Stepper<WSPoint> s(this);
  if(s.step()) return *s;
  return 0;
}

ZTrans* WSSeed::init_slide(WSPoint* f)
{
  if(f == 0) f = get_first_point();
  if(f == 0) return 0;
  return new ZTrans(f->RefTrans());
}

void WSSeed::ring(ZTrans& lcf, WSPoint* f, Double_t t)
{
  Double_t *Pnt = lcf.ArrT();
  Double_t *Axe = lcf.ArrX(), *Up = lcf.ArrY(), *Aw = lcf.ArrZ();

  const Double_t t2 = t*t, t3 = t2*t;

  for(Int_t i=0; i<3; i++) {
    Pnt[i] = f->mCoffs(i, 0) + f->mCoffs(i, 1)*t +
      f->mCoffs(i, 2)*t2 + f->mCoffs(i, 3)*t3;
    Axe[i] = f->mCoffs(i, 1) + 2*f->mCoffs(i, 2)*t +
      3*f->mCoffs(i, 3)*t2;
  }
  Double_t w = f->mCoffs(3, 0) + f->mCoffs(3, 1)*t +
    f->mCoffs(3, 2)*t2 + f->mCoffs(3, 3)*t3;
  Double_t dwdt = TMath::ATan(f->mCoffs(3, 1) + 2*f->mCoffs(3, 2)*t +
			     3*f->mCoffs(3, 3)*t2);
  Double_t dwc = TMath::Cos(dwdt);
  Double_t dws = TMath::Sin(dwdt);

  lcf.OrtoNorm3();

  pTuber->NewRing(mPLevel, true);
  Double_t phi = 0, step = 2*TMath::Pi()/mPLevel;
  Float_t R[3], N[3], T[2];
  for(int j=0; j<mPLevel; j++, phi-=step) {
    Double_t cp = TMath::Cos(phi), sp = TMath::Sin(phi);
    for(Int_t i=0; i<3; ++i) {
      R[i] = Pnt[i] + cp*w*Up[i] + sp*w*Aw[i];
      N[i] = -dws*Axe[i] + dwc*(cp*Up[i] + sp*Aw[i]);
    }
    // perhaps should invert normals for w<0
    T[0] = hTexU; T[1] = hTexV  - phi/TMath::TwoPi();
    pTuber->NewVert(R, N, 0, T);
  }
  { // last one
    phi = -2*TMath::Pi();
    Double_t cp = TMath::Cos(phi), sp = TMath::Sin(phi);
    for(Int_t i=0; i<3; ++i) {
      R[i] = Pnt[i] + cp*w*Up[i] + sp*w*Aw[i];
      N[i] = -dws*Axe[i] + dwc*(cp*Up[i] + sp*Aw[i]);
    }
    // perhaps should invert normals for w<0
    T[0] = hTexU; T[1] = hTexV + 1;
    pTuber->NewVert(R, N, 0, T);
  }

}

/**************************************************************************/

void WSSeed::Travel(Double_t abs_dt, UInt_t sleep_ms, Bool_t reverse_p)
{
  TRandom rndgen(0);

  Sphere* s = new Sphere("Observator");
  ZTrans trans;
  {
    GLensWriteHolder wrlck(this);
    ZTrans* tt = init_slide(0);
    trans = *tt;
    delete tt;
    if(reverse_p) {
      TransAtTime(trans, mLength, false);
      TVector3 xcy = trans.GetBaseVec(1);
      xcy = xcy.Cross(trans.GetBaseVec(2));
      TVector3 z(trans.GetBaseVec(3));
      if(xcy.Dot(z) < 0) {
	trans.SetBaseVec(3, -z);
      }
    }
    s->SetTrans(trans);
    s->SetUseScale(true);
    s->SetSx(0.2+0.8*rndgen.Rndm()); s->SetSy(0.2+0.8*rndgen.Rndm()); s->SetSz(0.2+0.8*rndgen.Rndm()); 
    s->SetColor(0.2+0.8*rndgen.Rndm(), 0.2+0.8*rndgen.Rndm(), 0.2+0.8*rndgen.Rndm());
  }

  {
    GLensWriteHolder q_wrlck(mQueen);
    mQueen->CheckIn(s);   // try-catch !!!!
  }

  {
    GLensWriteHolder s_wrlck(s);
    s->SetParent(this);
  }

  // This whole thing should be split into two functions for
  // proper operation in a cluster context.
  // 1. Instantiation at the Sun, Broadcast of MIR to call stepper in a
  //    dedicated thread. (declared as X{E}, require MIR)
  // 2. Stepper itself (declared as X{ED})


  {
    GLensWriteHolder wrlck(this);
    Add(s);
  }
  Double_t time, dt, max_t;
  if(!reverse_p) { time = 0;        dt =  abs_dt;  max_t = mLength; }
  else           { time = mLength;  dt = -abs_dt;  max_t = 0;  }
    

  while(1) {
    gSystem->Sleep(sleep_ms);
    time += dt;
    if(time*(reverse_p ? -1 : 1) > max_t) {
      {
	GLensWriteHolder s_wrlck(s);
	s->SetParent(0);
      }
      {
	GLensWriteHolder wrlck(this);
	// s->DecRefCount(this);
	RemoveAll(s);
	// mQueen->RemoveLens(s);
      }
      break;
    }
    {
      GLensWriteHolder wrlck(this);
      TransAtTime(trans, time, false);
    }
    {
      GLensWriteHolder wrlck(s);
      s->SetTrans(trans);
    }
  }
}
