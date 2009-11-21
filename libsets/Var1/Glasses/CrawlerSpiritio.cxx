// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "CrawlerSpiritio.h"
#include "Crawler.h"
#include "CrawlerSpiritio.c7"
#include <Stones/KeyHandling.h>

// CrawlerSpiritio

//______________________________________________________________________________
//
//

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
}

CrawlerSpiritio::CrawlerSpiritio(const Text_t* n, const Text_t* t) :
  ExtendioSpiritio(n, t)
{
  _init();
}

CrawlerSpiritio::~CrawlerSpiritio()
{}

//==============================================================================

Crawler* CrawlerSpiritio::get_crawler()
{
  return (Crawler*) *mExtendio;
}

//==============================================================================

void CrawlerSpiritio::TimeTick(Double_t t, Double_t dt)
{
  Crawler &C = * (Crawler*) *mExtendio;

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
