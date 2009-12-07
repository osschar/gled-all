// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "CrawlerSpiritio.h"
#include "Crawler.h"
#include <Glasses/AlSource.h>
#include "CrawlerSpiritio.c7"
#include <Glasses/Camera.h>
#include <Glasses/Tringula.h>
#include <Stones/KeyHandling.h>

// CrawlerSpiritio

//______________________________________________________________________________
//
// AlSource should be set to LT_CamDelta mode and buffer should be set.

ClassImp(CrawlerSpiritio);

//==============================================================================

#define KEY_CALLBACK(FOO) new KeyCallback<CrawlerSpiritio>(&CrawlerSpiritio::FOO)

void CrawlerSpiritio::_init()
{
  // From ExtendioSpiritio -- restric extendio fid.
  mExtendio_fid = Crawler::FID();

  using namespace KeyHandling;

  RegisterKey(KeyInfo("IncThrottle", "Increase throttle", KEY_CALLBACK(IncThrottle)));
  RegisterKey(KeyInfo("DecThrottle", "Decrease throttle", KEY_CALLBACK(DecThrottle)));

  RegisterKey(KeyInfo("LeftWheel",   "Turn wheel left",   KEY_CALLBACK(LeftWheel)));
  RegisterKey(KeyInfo("RightWheel",  "Turn wheel right",  KEY_CALLBACK(RightWheel)));

  RegisterKey(KeyInfo("FireGun",     "Fire gun",          KEY_CALLBACK(FireGun)));
}

CrawlerSpiritio::CrawlerSpiritio(const Text_t* n, const Text_t* t) :
  ExtendioSpiritio(n, t)
{
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

  mCameraBase->SetParent(mExtendio->GetTringula());
  mCameraBase->ref_trans().SetFromArray(mExtendio->ref_last_trans());

  mCamera->Home();

  // Move camera slightly off.
  // These factors seem to work ok with simple Crawlers.
  // Should really have a marked point in the mesh.
  // And draw something special when driving a unit.

  Float_t *minmax = mExtendio->GetMesh()->GetTTvor()->mMinMaxBox;
  mCamera->MoveLF(1, 0.20f * minmax[0]); // min_x
  mCamera->MoveLF(3, 1.30f * minmax[5]); // max_z
  mCamera->RotateLF(3, 1, 0.2);

  Crawler &C = * (Crawler*) *mExtendio;
  C.SetDriveMode(Crawler::DM_Controllers);

  if (*mEngineSrc)
  {
    mEngineSrc->Loop();
  }
}

void CrawlerSpiritio::Deactivate()
{
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
  Crawler &C = * (Crawler*) *mExtendio;

  mCameraBase->ref_trans().SetFromArray(mExtendio->ref_last_trans());

  // Throttle
  Int_t tdc = mKeyIncThrottle.fDownCount - mKeyDecThrottle.fDownCount;

  if (tdc)
  {
    const SDesireVarF& tv = C.RefThrottle();
    const Float_t ddt = (2.0 * tdc) * dt;
    Float_t d0 = tv.GetDesire();
    Float_t d1 = tv.DeltaDesire(tv.GetDelta()*ddt);
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
    const Float_t ddt = (2 * wdc) * dt;
    Float_t d0 = wv.GetDesire();
    Float_t d1 = wv.DeltaDesire(wv.GetDelta()*ddt);
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
      else
	tv.DeltaDesire(2.0f*tv.GetDelta());
    }
    else
    {
      ++mKeyIncThrottle.fDownCount;
    }
  }
  else
  {
    if (mKeyIncThrottle.fDownCount > 0)
      --mKeyIncThrottle.fDownCount;
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
      else
	tv.DeltaDesire(-2.0f*tv.GetDelta());
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
	wv.DeltaDesire(2.0f*wv.GetDelta());
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
	wv.DeltaDesire(-2.0f*wv.GetDelta());
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
  if (downp && *mGunSrc && ! mGunSrc->IsPlaying())
  {
    mGunSrc->Play();
  }
}
