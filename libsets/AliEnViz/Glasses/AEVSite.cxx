// $Header$

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

ClassImp(AEVSite);

/**************************************************************************/

void AEVSite::_init()
{
  mLatitude = mLongitude = 0;

  mSiteSize = 0; // expected number of cpus, 0 - 2500
  mJobsStarted = mJobsRunning = mJobsSaving = mJobsDone = mJobsError = 0;

  bBallOnStage = false;
}

/**************************************************************************/

/**************************************************************************/