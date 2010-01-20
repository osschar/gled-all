// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "CrawlerSpiritio.h"
#include "Crawler.h"
#include <Glasses/AlSource.h>
#include "CrawlerSpiritio.c7"

#include "TSPupilInfo.h"
#include "TringuRep.h"
#include "Tringula.h"
#include <Glasses/Camera.h>
#include <Stones/KeyHandling.h>

// CrawlerSpiritio

//______________________________________________________________________________
//
// AlSource should be set to LT_CamDelta mode and buffer should be set.

ClassImp(CrawlerSpiritio);

//==============================================================================

#define KEY_CALLBACK(FOO) new KeyHandling::KeyCallback<CrawlerSpiritio>(&CrawlerSpiritio::FOO)

void CrawlerSpiritio::_init()
{
  // From ExtendioSpiritio -- restric extendio fid.
  mExtendio_fid = Crawler::FID();
}

CrawlerSpiritio::CrawlerSpiritio(const Text_t* n, const Text_t* t) :
  ExtendioSpiritio(n, t),
  mKeyIncThrottle(RegisterKey("IncThrottle", "Increase throttle", KEY_CALLBACK(IncThrottle))),
  mKeyDecThrottle(RegisterKey("DecThrottle", "Decrease throttle", KEY_CALLBACK(DecThrottle))),
  mKeyLeftWheel  (RegisterKey("LeftWheel",   "Turn wheel left",   KEY_CALLBACK(LeftWheel))),
  mKeyRightWheel (RegisterKey("RightWheel",  "Turn wheel right",  KEY_CALLBACK(RightWheel)))
{
  RegisterKey("FireGun", "Fire gun", KEY_CALLBACK(FireGun));

  _init();
}

CrawlerSpiritio::~CrawlerSpiritio()
{}

//------------------------------------------------------------------------------

Crawler* CrawlerSpiritio::get_crawler()
{
  return (Crawler*) *mExtendio;
}

//==============================================================================

void CrawlerSpiritio::AdEnlightenment()
{
  // Create the camera.

  PARENT_GLASS::AdEnlightenment();

  if (mCameraBase == 0)
  {
    ZNode* cb = new ZNode("CrawlerCameraBase", "CameraBase of CrawlerSpiritio");
    cb->SetMIRActive(false);
    mQueen->CheckIn(cb);
    SetCameraBase(cb);
  }
  if (mCamera == 0)
  {
    Camera* c = new Camera("CrawlerCamera", "Camera of CrawlerSpiritio");
    c->SetMIRActive(false);
    mQueen->CheckIn(c);
    SetCamera(c);
    mCameraBase->Add(c);
  }
}

//==============================================================================

void CrawlerSpiritio::Activate()
{
  // Called when TSPupilInfo installs the spiritio.

  mCameraBase->SetParent(mPupilInfo->GetTringuRep());
  mCameraBase->ref_trans().SetFromArray(mExtendio->ref_last_trans());

  mCamera->Home();

  // Move camera slightly off.
  // These factors seem to work ok with simple Crawlers.
  // Should really have a marked point in the mesh.
  // And draw something special when driving a unit.

  Float_t *minmax = mExtendio->GetMesh()->GetTTvor()->mMinMaxBox;
  mCamera->Identity();
  mCamera->MoveLF(1, 0.20f * minmax[0]); // min_x
  mCamera->MoveLF(3, 1.30f * minmax[5]); // max_z
  mCamera->RotateLF(3, 1, 0.2);
  mCamera->SetHomeTrans();

  Crawler &C = * (Crawler*) *mExtendio;
  C.SetDriveMode(Crawler::DM_Controllers);

  if (*mEngineSrc)
  {
    mEngineSrc->Loop();
  }

  PARENT_GLASS::Activate();
}

void CrawlerSpiritio::Deactivate()
{
  PARENT_GLASS::Deactivate();

  if (*mEngineSrc)
  {
    mEngineSrc->Stop();
  }

  mCameraBase->SetParent(0);

  Crawler &C = * (Crawler*) *mExtendio;
  C.SetDriveMode(Crawler::DM_ConstVelocities);
}

//==============================================================================

void CrawlerSpiritio::TimeTick(Double_t t, Double_t dt)
{
  const Float_t dtf = dt;

  Crawler &C = * (Crawler*) *mExtendio;

  mCameraBase->ref_trans().SetFromArray(mExtendio->ref_last_trans());

  // Throttle
  Int_t tdc = mKeyIncThrottle.fDownCount - mKeyDecThrottle.fDownCount;

  if (tdc)
  {
    const SDesireVarF& tv = C.RefThrottle();
    Float_t d0 = tv.GetDesire();
    Float_t d1 = tv.DeltaDesire(tv.GetStdDesireDelta() * dtf * tdc);
    if (d0 < 0 && d1 > 0 && tdc > 0)
    {
      tv.SetDesire(0);
      --mKeyIncThrottle.fDownCount;
    }
    else if (d0 > 0 && d1 < 0 && tdc < 0)
    {
      tv.SetDesire(0);
      --mKeyDecThrottle.fDownCount;
    }
  }

  // Wheel
  Int_t wdc = mKeyLeftWheel.fDownCount - mKeyRightWheel.fDownCount;
  if (wdc)
  {
    const SDesireVarF& wv = C.RefWheel();
    Float_t d0 = wv.GetDesire();
    Float_t d1 = wv.DeltaDesire(wv.GetStdDesireDelta() * dtf * wdc);
    if (d0 < 0 && d1 > 0 && wdc > 0)
    {
      wv.SetDesire(0);
      --mKeyLeftWheel.fDownCount;
    }
    else if (d0 > 0 && d1 < 0 && wdc < 0)
    {
      wv.SetDesire(0);
      --mKeyRightWheel.fDownCount;
    }
  }

  if (*mEngineSrc)
  {
    const SDesireVarF& tv = C.RefThrottle();
    Float_t p = 0.2f + 0.8f*TMath::Abs(tv.Get())/tv.GetMax();
    if (p != mEngineSrc->GetPitch())
      mEngineSrc->SetPitch(p);
  }
}

//==============================================================================

void CrawlerSpiritio::IncThrottle(Int_t, Bool_t downp, UInt_t time_elapsed)
{
  Crawler &C = * (Crawler*) *mExtendio;

  if (downp)
  {
    if (time_elapsed < sDoubleClickTime)
    {
      const SDesireVarF& tv = C.RefThrottle();
      if (tv.IsAboveDesire())
	tv.DesireToValue();
      else if (tv.GetDesire() < 0)
	tv.SetDesire(0);
      else
	tv.SetDesire(tv.GetMax());
    }
    else
    {
      ++mKeyIncThrottle.fDownCount;
    }
  }
  else
  {
    if (mKeyIncThrottle.fDownCount > 0)
    {
      --mKeyIncThrottle.fDownCount;
    }
  }
}

void CrawlerSpiritio::DecThrottle(Int_t, Bool_t downp, UInt_t time_elapsed)
{
  Crawler &C = * (Crawler*) *mExtendio;

  if (downp)
  {
    if (time_elapsed < sDoubleClickTime)
    {
      const SDesireVarF& tv = C.RefThrottle();

      if (tv.IsBelowDesire())
	tv.DesireToValue();
      else if (tv.GetDesire() > 0)
	tv.SetDesire(0);
      else
	tv.SetDesire(tv.GetMin());
    }
    else
    {
      ++mKeyDecThrottle.fDownCount;
    }
  }
  else
  {
    if (mKeyDecThrottle.fDownCount > 0)
      --mKeyDecThrottle.fDownCount;
  }
}

void CrawlerSpiritio::SetThrottle(Float_t t)
{
  Crawler &C = * (Crawler*) *mExtendio;

  C.RefThrottle().SetDesire(t);
}

//------------------------------------------------------------------------------

void CrawlerSpiritio::LeftWheel(Int_t, Bool_t downp, UInt_t time_elapsed)
{
  Crawler &C = * (Crawler*) *mExtendio;

  if (downp)
  {
    if (time_elapsed < sDoubleClickTime)
    {
      const SDesireVarF& wv = C.RefWheel();

      if (wv.GetDesire() < 0)
	wv.SetDesire(0);
      else
	wv.DeltaDesire(wv.GetStdDesireDelta());
    }
    else
    {
      ++mKeyLeftWheel.fDownCount;
    }
  }
  else
  {
    if (mKeyLeftWheel.fDownCount > 0)
      --mKeyLeftWheel.fDownCount;
  }
}

void CrawlerSpiritio::RightWheel(Int_t, Bool_t downp, UInt_t time_elapsed)
{
  Crawler &C = * (Crawler*) *mExtendio;

  if (downp)
  {
    if (time_elapsed < sDoubleClickTime)
    {
      const SDesireVarF& wv = C.RefWheel();

      if (wv.GetDesire() > 0)
	wv.SetDesire(0);
      else
	wv.DeltaDesire(-wv.GetStdDesireDelta());
    }
    else
    {
      ++mKeyRightWheel.fDownCount;
    }
  }
  else
  {
    if (mKeyRightWheel.fDownCount > 0)
      --mKeyRightWheel.fDownCount;
  }
}

void CrawlerSpiritio::SetWheel(Float_t w)
{
  Crawler &C = * (Crawler*) *mExtendio;

  C.RefWheel().SetDesire(w);
}

//------------------------------------------------------------------------------

void CrawlerSpiritio::FireGun(Int_t, Bool_t downp, UInt_t time_elapsed)
{
  Crawler &C = * (Crawler*) *mExtendio;

  if (downp && C.RefLaserCharge().Get() > 20.0f)
  {
    C.ShootLaser();
  }
}
