// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// GuiPupilInfo
//
//

#include "GuiPupilInfo.h"
#include "GuiPupilInfo.c7"

ClassImp(GuiPupilInfo)

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


/**************************************************************************/
