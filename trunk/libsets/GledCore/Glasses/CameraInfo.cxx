// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// CameraInfo
//
// Copy of PupilInfo's camera information.

// Should be solved in some other way ... say via p7 supporting inclusion of
// stone or non-glass-base members together with widget specification.

#include "CameraInfo.h"
#include "CameraInfo.c7"

ClassImp(CameraInfo)

/**************************************************************************/

void CameraInfo::_init()
{
  bFixCameraBase = bFixLookAt = bFixUpReference = true;


  mCameraBase    = 0;

  mLookAt        = 0;
  mLookAtMinDist = 0.1;

  mUpReference   = 0;
  mUpRefAxis     = 3;
  bUpRefLockDir  = true;
  mUpRefMinAngle = 10;

  mProjMode = P_Perspective;
  mZFov     = 90;   mZSize   = 20;
  mYFac     = 1;    mXDist   = 10;
  mNearClip = 0.01; mFarClip = 100;
}

/**************************************************************************/


/**************************************************************************/
