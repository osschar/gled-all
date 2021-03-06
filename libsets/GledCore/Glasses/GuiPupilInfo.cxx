// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// GuiPupilInfo
//
//

#include "GuiPupilInfo.h"
#include "GuiPupilInfo.c7"
#include <Glasses/ZQueen.h>

ClassImp(GuiPupilInfo);

/**************************************************************************/

void GuiPupilInfo::_init()
{
  // Override from SubShellInfo:
  mCtorLibset = "GledCore";
  mCtorName   = "GuiPupil";

  mPupil   = 0;
  mCameras = 0;
}

/**************************************************************************/

void GuiPupilInfo::AssertDefaultPupil()
{
  if(mPupil == 0) {
    PupilInfo* p = new PupilInfo("Swallowed Pupil", GForm("Pupil of %s", GetName()));
    mQueen->CheckIn(p);
    SetPupil(p);
  }
}

/**************************************************************************/
