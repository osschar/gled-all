// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// Text
//
//

#include "Text.h"
#include "Text.c7"

ClassImp(Text)

/**************************************************************************/

void Text::_init()
{
  // !!!! Set all links to 0 !!!!
  mFont = 0;

  // Override settings from ZGlass
  bUseDispList = true;
  // Override settings from ZNode
  bUseScale = true;

  bAlpha     = false;   bBlend     = true;
  bPolyOffFG = true;    bPolyOffBG = true;
  mPOFac     = -2;      mPOUni     = -2;
  bAbsSize   = true;    bCenter    = true;
  bFillBack  = false;
  bBorder    = true;
  mXBorder   = 0.1;     mYBorder   = 0.1;
  mFGCol.rgba(1,1,1,1); mBGCol.rgba(0,0,0,1);
}

/**************************************************************************/
