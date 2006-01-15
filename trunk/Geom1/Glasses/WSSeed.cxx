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

#include <TF1.h>

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
  mTexUScale  = mTexVScale  = 1;

  mTrueLength = 0; bRenormLen = false; mLength = 1;

  mLineW = 2; bFat = true;

  mTexture = 0;

  mDtexU = 0;      mDtexV = 0;

  bAnimRunning = false;
  mAnimSleepMS = 50;
  mAnimTime    = 0;
  mAnimStepFac = 1;

  pTuber = 0;     bTextured = false;
  m_first_point = m_last_point = 0; m_num_points = 0;
}

WSSeed::~WSSeed() { delete pTuber; }

/**************************************************************************/
/**************************************************************************/

void WSSeed::on_insert(iterator it)
{
  PARENT_GLASS::on_insert(it);

  WSPoint* m = dynamic_cast<WSPoint*>(it->fLens);
  if(m != 0) {
    Float_t delta_len = 0;
    iterator i = it; ++i;
    WSPoint* n = 0;
    while(i != end()) {
      n = dynamic_cast<WSPoint*>((i++)->fLens);
      if(n != 0) {
	n->mPrevPoint = m;
	m->mNextPoint = n;
	delta_len += m->mStretch;
	break;
      }
    }
    if(n == 0)
      m_last_point  = m;
    
    WSPoint* p = 0;
    while(it != begin()) {
      p = dynamic_cast<WSPoint*>((--it)->fLens);
      if(p != 0) {
	m->mPrevPoint = p;
	if(p->mNextPoint == 0)
	  delta_len += p->mStretch;
	p->mNextPoint = m;
	break;
      } else {
	m_first_point = m;
      }
    }
    if(p == 0)
      m_first_point  = m;

    ++m_num_points;
    mTrueLength += delta_len;

    mStampReqTring = Stamp(FID());
  }
}

void WSSeed::on_remove(iterator it)
{
  PARENT_GLASS::on_remove(it);

  WSPoint* m = dynamic_cast<WSPoint*>(it->fLens);
  if(m != 0) {
    Float_t delta_len = 0;
    if(m->mPrevPoint) {
      if(m->mNextPoint == 0)
	delta_len -= m->mPrevPoint->mStretch;
      m->mPrevPoint->mNextPoint = m->mNextPoint;
    } else {
      m_first_point = m->mNextPoint;
    }
    if(m->mNextPoint) {
      m->mNextPoint->mPrevPoint = m->mPrevPoint;
      delta_len -= m->mStretch;
    } else {
      m_last_point  = m->mPrevPoint;
    }
    m->mPrevPoint = 0;
    m->mNextPoint = 0;
    
    --m_num_points;
    mTrueLength += delta_len;

    mStampReqTring = Stamp(FID());
  }
}

void WSSeed::on_rebuild()
{
  PARENT_GLASS::on_rebuild();

  mTrueLength = 0;
  m_first_point = m_last_point = 0; m_num_points = 0;
  WSPoint *p = 0, *n = 0;
  for(iterator i=begin(); i!=end(); ++i) {
    n = dynamic_cast<WSPoint*>(i->fLens);
    if(n != 0) {      
      if(p != 0) {
	n->mPrevPoint = p;
	p->mNextPoint = n;
	mTrueLength += p->mStretch;
      } else {
	m_first_point = n;
      }
      m_last_point = p = n;
      ++m_num_points;
    }
  }
  mStampReqTring = Stamp(FID());
}

void WSSeed::clear_list()
{
  PARENT_GLASS::clear_list();

  mTrueLength = 0;
  m_first_point = m_last_point = 0; m_num_points = 0;

  mStampReqTring = Stamp(FID());
}

/**************************************************************************/

void WSSeed::Triangulate()
{
  // Should be called under ReadLock.

  if(m_num_points < 2) {
    delete pTuber; pTuber = 0;
    return;
  }
  if(!bFat) {
    // !!!!! Could at least call Coffs !!!!!
    // Then don't need them in line rnr
    // and in TransAtTime
    return;
  }

  bTextured = (mTexture != 0);
  if(pTuber == 0) pTuber = new TubeTvor;
  pTuber->Init(0, mTLevel*(m_num_points - 1) + 1, mPLevel, false, bTextured);

  Float_t len_fac = (bRenormLen) ? mLength / mTrueLength : 1;

  bool first = true;
  WSPoint *a, *b;
  b = m_first_point;
  ZTrans* lcfp;
  while((a=b, b=b->mNextPoint, b) != 0) {
    a->Coff(b);
    if(first) {
      first = false;
      lcfp  = init_slide(a);
      hTexU = 0;
      hTexV = 0;
    }
    Float_t delta = 1.0/mTLevel, max = 1 - delta/2;
    for(Float_t t=0; t<max; t+=delta) {
      ring(*lcfp, a, t);
      hTexU += delta * a->mStretch * len_fac;
      hTexV += delta * a->mTwist;
    }
  }
  ring(*lcfp, a->mPrevPoint, 1);
  delete lcfp;
}

/**************************************************************************/
/**************************************************************************/

Float_t WSSeed::Length()
{
  return bRenormLen ? mLength : mTrueLength;
}

Float_t WSSeed::MeasureLength()
{
  GMutexHolder lstlck(mListMutex);

  Float_t len = 0;
  if(m_num_points >= 2) {
    WSPoint* a = m_first_point;
    while(a->mNextPoint != 0) {
      len += a->mStretch;
      a = a->mNextPoint;
    }
  }
  if(len != mTrueLength)
    SetTrueLength(len);
  return len;
}

void WSSeed::MeasureAndSetLength()
{
  mLength = MeasureLength();
  mStampReqTring = Stamp(FID());
}

/**************************************************************************/
/**************************************************************************/

void WSSeed::TransAtTime(ZTrans& lcf, Double_t time, Bool_t repeat_p, Bool_t reinit_trans_p)
{
  // Positions lcf to time.
  // If repeat_p is true, the time is clamped into the range.
  // 'Up' and 'right' axes are Gram-Schmidt orto-normalized wrt 'front'.

  static const Exc_t _eh("WSSeed::TransAtTime ");

  if(m_num_points < 2)
    throw(_eh + "nedd at least two points.");

  // Here should call triangulate if mStamReqTex < mTimeStamp
  // The triangulation could measure lengths etc, store them in local data.
  // Then also don't need Coff call below.
  // Coff is now optimised.

  Double_t len_fac = 1;
  Double_t len     = mTrueLength;
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
  WSPoint *a, *b;
  b = m_first_point;
  while((a=b, b=b->mNextPoint, b) != 0) {
    done += a->mStretch * len_fac;
    if(done >= time) break;
  }
  Double_t t;
  if(b != 0) {
    t = 1  -  (done - time) / (a->mStretch * len_fac);
  } else {
    t = 1;
    b = a; a = b->mPrevPoint;
  }
  a->Coff(b);

  if(reinit_trans_p)
    lcf = a->RefTrans();

  const Double_t t2 = t*t, t3 = t2*t;
  Double_t* Pnt = lcf.ArrT();
  Double_t* Axe = lcf.ArrX();

  for(Int_t i=0; i<3; i++) {
    const TMatrixDRow R( a->mCoffs[i] );
    Pnt[i] = R[0] +   R[1]*t +   R[2]*t2 + R[3]*t3;
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
// Animation, TimeTick
/**************************************************************************/

void WSSeed::StartAnimation()
{
  if(bAnimRunning) return;

  {
    GLensWriteHolder wlck(this);
    bAnimRunning = true;
    Stamp(FID());
  }
  m_anim_tick = 0;
  Double_t dt = 0;
  while(bAnimRunning) {
    TimeTick(mAnimTime, dt);
    dt = 0.001*mAnimSleepMS*mAnimStepFac;
    GTime::SleepMiliSec(mAnimSleepMS);
    mAnimTime += dt;
    ++m_anim_tick;
  }
}

void WSSeed::StopAnimation()
{
  if(!bAnimRunning) return;
  bAnimRunning = false;
  Stamp(FID());
}

/**************************************************************************/
/**************************************************************************/

void WSSeed::TimeTick(Double_t t, Double_t dt)
{
  GLensWriteHolder wlck(this);
  if(mDtexU != 0 || mDtexV != 0) {
    mTexUOffset += dt*mDtexU;
    mTexVOffset += dt*mDtexV;
    if(m_anim_tick % 1000 == 0) {
      if(mTexUOffset >  100) mTexUOffset -= 200;
      if(mTexUOffset < -100) mTexUOffset += 200;
      if(mTexVOffset >  100) mTexVOffset -= 200;
      if(mTexVOffset < -100) mTexVOffset += 200;
    }
    Stamp(FID());
  }
}

/**************************************************************************/
/**************************************************************************/



void WSSeed::MakeLissajou(Double_t t_min, Double_t t_max, Int_t n_points,
			  Double_t ax, Double_t wx, Double_t dx,
			  Double_t ay, Double_t wy, Double_t dy,
			  Double_t az, Double_t wz, Double_t dz,
			  Float_t def_width)
{
  // Creates a WeaverSymbol following a Lissajou curve.
  // Each coordinate specified as: x(t) = ax * sin(wx * t + dx).

  ClearList();

  using namespace TMath;

  Double_t step = (t_max - t_min) / (n_points - 1);
  Double_t t    = t_min;

  ZTrans trans;
  for(Int_t i=1; i<=n_points; ++i, t+=step) {
    if(i == n_points) t = t_max;

    Double_t pos_x = ax*Sin(wx*t + dx);
    Double_t pos_y = ay*Sin(wy*t + dy);
    Double_t pos_z = az*Sin(wz*t + dz);
    Double_t der_x = ax*wx*Cos(wx*t + dx);
    Double_t der_y = ay*wy*Cos(wy*t + dy);
    Double_t der_z = az*wz*Cos(wz*t + dz);

    trans.SetPos(pos_x, pos_y, pos_z);
    trans.SetBaseVec(1, der_x, der_y, der_z);
    trans.OrtoNorm3();

    WSPoint* p = new WSPoint(GForm("Point %d", i));
    p->SetTrans(trans);
    p->mW = def_width;

    mQueen->CheckIn(p);
    Add(p);
  }

  mStampReqTring = Stamp(FID());
}

void WSSeed::MakeFromFormulas(Double_t t_min, Double_t t_max, Int_t n_points,
			      TString fx, TString fy, TString fz,
			      Float_t def_width)
{
  // Creates a WeaverSymbol following formulas for x, y and z.
  // The name of the time variable in formulas is 'x' and thus one
  // should write, eg: "0.5*x^2".

  ClearList();

  using namespace TMath;

  Double_t step = (t_max - t_min) / (n_points - 1);
  Double_t t    = t_min;

  TF1 tfx(GForm("WSSeed_x_%d", GetSaturnID()), fx, t_min, t_max);
  TF1 tfy(GForm("WSSeed_y_%d", GetSaturnID()), fy, t_min, t_max);
  TF1 tfz(GForm("WSSeed_z_%d", GetSaturnID()), fz, t_min, t_max);

  ZTrans trans;
  for(Int_t i=1; i<=n_points; ++i, t+=step) {
    if(i == n_points) t = t_max;

    Double_t pos_x = tfx.Eval(t);
    Double_t pos_y = tfy.Eval(t);
    Double_t pos_z = tfz.Eval(t);
    Double_t der_x = tfx.Derivative(t);
    Double_t der_y = tfy.Derivative(t);
    Double_t der_z = tfz.Derivative(t);

    trans.SetPos(pos_x, pos_y, pos_z);
    trans.SetBaseVec(1, der_x, der_y, der_z);
    trans.OrtoNorm3();

    WSPoint* p = new WSPoint(GForm("Point %d", i));
    p->SetTrans(trans);
    p->mW = def_width;

    mQueen->CheckIn(p);
    Add(p);
  }

  mStampReqTring = Stamp(FID());
}
