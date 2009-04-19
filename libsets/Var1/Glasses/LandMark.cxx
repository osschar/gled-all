// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "LandMark.h"
#include "LandMark.c7"

#include "Tringula.h"
#include "ParaSurf.h"

// LandMark

//______________________________________________________________________________
//
//

ClassImp(LandMark);

//==============================================================================

void LandMark::_init()
{
  mF = mG = mH = mHRel = 0;
  mHeightMode = HM_Absolute;
  mPhi = 0;
  mSx = mSy = 1;
  mColor.rgba(1, 0, 0.3, 0.5);

  bUseDispList = true;
}

LandMark::LandMark(const Text_t* n, const Text_t* t) :
  Extendio(n, t)
{
  _init();
}

LandMark::~LandMark()
{}

//==============================================================================

void LandMark::retrans(ParaSurf* ps)
{
  ps->regularize_fg(&mF);
  ps->fgh2trans(&mF, mTrans);
  mTrans.RotateLF(1, 2, TMath::DegToRad()*mPhi);
}

void LandMark::SetF(Float_t f)
{
  ParaSurf* ps = mTringula->GetParaSurf();

  if (f < ps->GetMinF()) f = ps->GetMinF();
  if (f > ps->GetMaxF()) f = ps->GetMaxF();
  mF = f;

  retrans(ps);
  Stamp();
}

void LandMark::SetG(Float_t g)
{
  ParaSurf* ps = mTringula->GetParaSurf();

  if (g < ps->GetMinG()) g = ps->GetMinG();
  if (g > ps->GetMaxG()) g = ps->GetMaxG();
  mG = g;

  retrans(ps);
  Stamp();
}

void LandMark::SetH(Float_t h)
{
  ParaSurf* ps = mTringula->GetParaSurf();

  mH = h;

  retrans(ps);
  Stamp();
}

void LandMark::SetHRel(Float_t hr)
{
  ParaSurf* ps = mTringula->GetParaSurf();

  mHRel = hr;

  retrans(ps);
  Stamp();
}

void LandMark::SetFGH(Float_t f, Float_t g, Float_t h)
{
  ParaSurf* ps = mTringula->GetParaSurf();

  if (f < ps->GetMinF()) f = ps->GetMinF();
  if (f > ps->GetMaxF()) f = ps->GetMaxF();
  mF = f;

  if (g < ps->GetMinG()) g = ps->GetMinG();
  if (g > ps->GetMaxG()) g = ps->GetMaxG();
  mG = g;

  mH = h;

  retrans(ps);
  Stamp(); 
}

void LandMark::SetPhi(Float_t p)
{
  ParaSurf* ps = mTringula->GetParaSurf();

  if (p < 0) p = 360 + p;
  if (p > 360) p = p - TMath::Floor(p / 360)*360;
  mPhi = p;

  retrans(ps);
  Stamp();
}
