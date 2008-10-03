// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AEVManager.h"
#include <Glasses/SolarSystem.h>
#include "AEVSite.h"
#include "AEVSiteList.h"
#include "AEVMlSucker.h"
#include "AEVManager.c7"

#include <Glasses/CosmicBall.h>

#include <Glasses/ZQueen.h>

#include <TMath.h>

namespace TMath
{
template <typename T> T Sqr(T x) { return x*x; }
}

// AEVManager

//______________________________________________________________________________
//
//

ClassImp(AEVManager);

//==============================================================================

void AEVManager::_init()
{
  mTimeWindow = 0;

  mSiteVizMode = SV_All;
  mSiteMinVal = mSiteMaxVal = 0;
  mSiteMinCut = mSiteMaxCut = 0;

  mSVJobsRunning.fVarPtr = &AEVSite::mJobsRunning;
  mSVJobsError  .fVarPtr = &AEVSite::mJobsError;
  mSVCurrent = 0;

  mUserVizMode = UV_None;
}

AEVManager::AEVManager(const Text_t* n, const Text_t* t) :
  ZNameMap(n, t)
{
  _init();
}

AEVManager::~AEVManager()
{}

//==============================================================================

void AEVManager::SetTimeWindow(Int_t time)
{
  static const Exc_t _eh("AEVManager::SetTimeWindow ");

  assert_sucker(_eh);

  if (time < 0) time = 0;

  mSucker->SendLine(GForm("timewindow %d\n", time));

  GLensWriteHolder wrlck(this); 
  mTimeWindow = time;
  Stamp(FID());
}

void AEVManager::RescanLimits(Bool_t drop_cuts)
{
  if (mSVCurrent && drop_cuts)
  {
    mSVCurrent->fLow  = mSVCurrent->fMin;
    mSVCurrent->fHigh = mSVCurrent->fMax;
  }
  SetSiteVizMode(mSiteVizMode);
}

//==============================================================================

void AEVManager::update_sv_current()
{
  // Sites must have at least one element ... check before calling.

  SiteInt_t &bar = mSVCurrent->fVarPtr;
  Int_t     &min = mSVCurrent->fMin, &max  = mSVCurrent->fMax;
  Int_t     &low = mSVCurrent->fLow, &high = mSVCurrent->fHigh;

  bool  low_was_min = (low  == min);
  bool high_was_max = (high == max);

  Stepper<AEVSite> s(*mSites);
  s.step();
  min = max = (*s)->*bar;
  while (s.step())
  {
    min = TMath::Min(min, (*s)->*bar);
    max = TMath::Max(max, (*s)->*bar);
  }
  if (low_was_min  || low  < min)
    low = min;
  if (high_was_max || high > max)
    high = max;

  mSiteMinVal = min; mSiteMaxVal = max;
  mSiteMinCut = low; mSiteMaxCut = high;
}

void AEVManager::select_sv_current()
{
  const SiteInt_t &bar = mSVCurrent->fVarPtr;
  const Int_t     &low = mSVCurrent->fLow, &high = mSVCurrent->fHigh;

  const Float_t minR = mSolarSystem->GetOrbitMinR();
  const Float_t maxR = mSolarSystem->GetOrbitMaxR();
  const Float_t dFac = (maxR - minR) / TMath::Sqr(high - low);

  Stepper<AEVSite> s(*mSites);
  while (s.step())
  {
    if ((*s)->*bar < low || (*s)->*bar > high)
    {
      if (s->bBallOnStage)
      {
	mSolarSystem->RemovePlanetoid(*s->mBallViz);
	s->bBallOnStage = false;
      }
    }
    else
    {
      if ( ! s->bBallOnStage)
      {
	mSolarSystem->AddPlanetoid(*s->mBallViz);
	s->bBallOnStage = true;
      }
      s->GetBallViz()->SetDesiredR(minR + dFac*TMath::Sqr(high - (*s)->*bar));
    }
  }
}

//------------------------------------------------------------------------------

bool AEVManager::update_sv_current(AEVSite* site)
{
  // A site has changed ... does this influence current state?

  bool changed = false;

  SiteInt_t &bar = mSVCurrent->fVarPtr;
  Int_t     &min = mSVCurrent->fMin, &max  = mSVCurrent->fMax;
  Int_t     &low = mSVCurrent->fLow, &high = mSVCurrent->fHigh;

  bool  low_was_min = (low  == min);
  bool high_was_max = (high == max);

  if (site->*bar < min) { min = site->*bar; changed = true; }
  if (site->*bar > max) { max = site->*bar; changed = true; }

  if (changed)
  {
    if (low_was_min  || low  < min)
      low = min;
    if (high_was_max || high > max)
      high = max;

    mSiteMinVal = min; mSiteMaxVal = max;
    mSiteMinCut = low; mSiteMaxCut = high;
  }

  return changed;
}

void AEVManager::select_sv_current(AEVSite* site)
{
  const SiteInt_t &bar = mSVCurrent->fVarPtr;
  const Int_t     &low = mSVCurrent->fLow, &high = mSVCurrent->fHigh;

  const Float_t minR = mSolarSystem->GetOrbitMinR();
  const Float_t maxR = mSolarSystem->GetOrbitMaxR();
  const Float_t dFac = (maxR - minR) / TMath::Sqr(high - low);

  if (site->*bar < low || site->*bar > high)
  {
    if (site->bBallOnStage)
    {
      mSolarSystem->RemovePlanetoid(*site->mBallViz);
      site->bBallOnStage = false;
    }
  }
  else
  {
    if ( ! site->bBallOnStage)
    {
      mSolarSystem->AddPlanetoid(*site->mBallViz);
      site->bBallOnStage = true;
    }
    site->GetBallViz()->SetDesiredR(minR + dFac*TMath::Sqr(high - site->*bar));
  }
}

//==============================================================================

void AEVManager::EmitSiteVizRay()
{
  static const Exc_t _eh("AEVManager::EmitSiteVizRay ");

  assert_solarsystem(_eh);
  assert_sites(_eh);

  switch (mSiteVizMode)
  {
    case SV_None:
    {
      Stepper<AEVSite> s(*mSites);
      while (s.step())
      {
	if (s->bBallOnStage)
	{
	  mSolarSystem->RemovePlanetoid(*s->mBallViz);
	  s->bBallOnStage = false;
	}
      }
      mSVCurrent  = 0;
      mSiteMinVal = mSiteMaxVal = 0;
      mSiteMinCut = mSiteMaxCut = 0;
      break;
    }
    case SV_All:
    {
      Stepper<AEVSite> s(*mSites);
      while (s.step())
      {
	if ( ! s->bBallOnStage)
	{
	  mSolarSystem->AddPlanetoid(*s->mBallViz);
	  s->bBallOnStage = true;
	}
      }
      mSVCurrent  = 0;
      mSiteMinVal = mSiteMaxVal = 0;
      mSiteMinCut = mSiteMaxCut = 0;
      break;
    }
    case SV_RunningJobs:
    {
      if (mSites->Size() == 0)
      {
	mSiteVizMode = SV_All;
	break;
      }

      mSVCurrent = &mSVJobsRunning;
      update_sv_current();
      select_sv_current();
      break;
    }
    case SV_ErrorJobs:
    {
      if (mSites->Size() == 0)
      {
	mSiteVizMode = SV_All;
	break;
      }

      mSVCurrent = &mSVJobsError;
      update_sv_current();
      select_sv_current();
      break;
    }
  }
}

void AEVManager::EmitUserVizRay()
{

}

//==============================================================================


void AEVManager::SetSiteMinCut(Int_t min)
{
  if (min < mSiteMinVal)
    min = mSiteMinVal;
  if (min > mSiteMaxCut)
    min = mSiteMaxCut = TMath::Min(min, mSiteMaxVal);
  mSiteMinCut = min;

  if (mSVCurrent)
  {
    mSVCurrent->fLow  = mSiteMinCut;
    mSVCurrent->fHigh = mSiteMaxCut;
    select_sv_current();
  }

  Stamp(FID());
}

void AEVManager::SetSiteMaxCut(Int_t max)
{
  if (max > mSiteMaxVal)
    max = mSiteMaxVal;
  if (max < mSiteMinCut)
    max = mSiteMinCut = TMath::Max(max, mSiteMinVal);
  mSiteMaxCut = max;

  if (mSVCurrent)
  {
    mSVCurrent->fLow  = mSiteMinCut;
    mSVCurrent->fHigh = mSiteMaxCut;
    select_sv_current();
  }

  Stamp(FID());
}

//==============================================================================

void AEVManager::SiteNew(AEVSite* site)
{
  // This is called when a new site is added.
  // Should be called under write-lock.

  if (site->GetBallViz() == 0)
  {
    CosmicBall* cb = mSolarSystem->RandomPlanetoid(site->GetName());

    // Hack assuming max site-size ~2500.
    Double_t r_planet = TMath::Max(mSolarSystem->GetPlanetMinR(),
       TMath::Power(site->GetSiteSize(), 0.3333) / 15 * mSolarSystem->GetPlanetMaxR());
    Double_t mass_planet = 4.0*TMath::Pi()*r_planet*r_planet*r_planet/3.0;

    cb->SetM(mass_planet);
    cb->SetRadius(r_planet);

    cb->SetColor(gRandom->Uniform(0.4, 0.8),
		 gRandom->Uniform(0.4, 1.0),
		 gRandom->Uniform(0.4, 1.0));

    mQueen->CheckIn(cb);
    site->SetBallViz(cb);
  }

  switch (mSiteVizMode)
  {
    case SV_None:
      break;
    case SV_All:
      mSolarSystem->AddPlanetoid(*site->mBallViz);
      site->bBallOnStage = true;
      break;
    default:
      select_sv_current(site);
      break;
  }
}

void AEVManager::SitePositionChanged(AEVSite* site)
{
  // Relevant if we add mapviz
  // Should be called under write-lock.
}

void AEVManager::SiteChanged(AEVSite* site)
{
  // Do magick depending on site-viz-mode.
  // Should be called under write-lock.

  static const Exc_t _eh("AEVManager::ChangedSite ");

  assert_solarsystem(_eh);

  if (mSVCurrent)
  {
    if (update_sv_current(site))
    {
      select_sv_current();
      Stamp(FID());
    }
  }
}
