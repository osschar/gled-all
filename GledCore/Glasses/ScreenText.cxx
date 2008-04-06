// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ScreenText
//
// Displays mText in window coordinates. This class only contains the
// instructions, actual work is done by renderers.

#include "ScreenText.h"
#include "ScreenText.c7"

ClassImp(ScreenText)

/**************************************************************************/

void ScreenText::_init()
{
  // *** Set all links to 0 ***
  mBgCol.rgba(0,0,0,0.5);
  mX = 0; mY = 0; mZ = 1e-3;
}

/**************************************************************************/


/**************************************************************************/
