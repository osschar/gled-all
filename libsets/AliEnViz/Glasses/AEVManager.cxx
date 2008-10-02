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

// AEVManager

//______________________________________________________________________________
//
//

ClassImp(AEVManager);

//==============================================================================

void AEVManager::_init()
{
  mSiteVizMode = SV_All;
  mSiteMinVal = mSiteMaxVal = 0;
  mSiteMinCut = mSiteMaxCut = 0;

  mSVJobsRunning.fVarPtr = &AEVSite::mJobsRunning;
  mSVJobsError  .fVarPtr = &AEVSite::mJobsErrorAll;
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

void AEVManager::update_sv_current()
{
  // Sites must have at least one element ... check before calling.

  SiteInt_t &bar = mSVCurrent->fVarPtr;
  Int_t     &min = mSVCurrent->fMin, &max  = mSVCurrent->fMax;
  Int_t     &low = mSVCurrent->fLow, &high = mSVCurrent->fHigh;

  Stepper<AEVSite> s(*mSites);
  s.step();
  min = max = (*s)->*bar;
  while (s.step())
  {
    min = TMath::Min(min, (*s)->*bar);
    max = TMath::Max(max, (*s)->*bar);
  }
  if (low < min)
    low = min;
  if (high == 0 || high > max)
    high = max;

  mSiteMinVal = min; mSiteMaxVal = max;
  mSiteMinCut = low; mSiteMaxCut = high;
}

void AEVManager::select_sv_current()
{
  SiteInt_t &bar = mSVCurrent->fVarPtr;
  Int_t     &low = mSVCurrent->fLow, &high = mSVCurrent->fHigh;

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
    }
  }
}

//------------------------------------------------------------------------------

void AEVManager::update_sv_current(AEVSite* site)
{

}

void AEVManager::select_sv_current(AEVSite* site)
{

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
    max = mSiteMinVal;
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
    mQueen->CheckIn(cb);
    site->SetBallViz(cb);

    // !!!! This must go elsewhere ... check conditions etc
    // but probably unknow state anyway.
    mSolarSystem->AddPlanetoid(cb);
    site->bBallOnStage = true;
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

  if (mSiteVizMode == SV_None)
  {

  }
  else if (mSiteVizMode == SV_All)
  {
  }
  else
  {
    // Run selection somehow, decide what to do.
  }
}
