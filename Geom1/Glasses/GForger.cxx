// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// GForger
//
//

#include "GForger.h"
#include "GForger.c7"

#include <TSystem.h>

ClassImp(GForger)

/**************************************************************************/

void GForger::_init()
{
  mZFactor = 1;

  mFileName = "gforge.png";

  mMesh  = 256;
  mPower = 1.2;

  mDimension = 2.15;

  bCraters       = false;
  mCraterDensity = 1;
  mCraterHeight  = 0.5;
}

/**************************************************************************/

void GForger::Forge()
{
  lStr_t parts;
  parts.push_back
    (GForm("gforge -name %s -mesh %d -power %f -dimension %f %s",
	   mFileName.Data(), mMesh, mPower, mDimension, mADimText.Data()
	   ));
  if(bCraters)
    parts.push_back
      (GForm("-craters %f %f", mCraterDensity, mCraterHeight));

  gSystem->Exec(GledNS::join_strings(" ", parts));

  if(mImage != 0) {
    {
      GLensWriteHolder imglock(*mImage);
      mImage->SetFile(mFileName);
      mImage->Load();
    }
    if(mTerrain != 0) {
      GLensWriteHolder imglock(*mTerrain);
      mTerrain->SetFromImage(*mImage, mZFactor);
    }
  }
}

/**************************************************************************/
