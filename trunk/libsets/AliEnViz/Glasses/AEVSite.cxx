// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AEVSite
//
//

#include "AEVSite.h"
#include <Glasses/CosmicBall.h>
#include "AEVSite.c7"

#include <TMath.h>

ClassImp(AEVSite);

/**************************************************************************/

void AEVSite::_init()
{
  mLatitude = mLongitude = 0;

  mSiteSize = 0; // expected number of cpus, 0 - 2500
  mJobsStarted = mJobsRunning = mJobsSaving = mJobsDone = mJobsError = 0;
  mEventsAll = mEventsDone = mEventsFrac = 0;

  bBallOnStage = false;
}

/**************************************************************************/

void AEVSite::set_event_frac()
{
  if (mEventsAll > 0)
    mEventsFrac = TMath::Nint(1000.0 * mEventsDone / mEventsAll);
  else
    mEventsFrac = 1000;
}

void AEVSite::SetEventsAll(Int_t ea)
{
  mEventsAll = ea;
  set_event_frac();
  Stamp(FID());
}

void AEVSite::SetEventsDone(Int_t ed)
{
  mEventsDone = ed;
  set_event_frac();
  Stamp(FID());
}

/**************************************************************************/
