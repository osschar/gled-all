// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZRlNodeMarkup
//
//

#include "ZRlNodeMarkup.h"
#include "ZRlNodeMarkup.c7"

ClassImp(ZRlNodeMarkup)

/**************************************************************************/

void ZRlNodeMarkup::_init()
{
  mNodeMarkupOp  = O_On;

  bRnrAxes    = false;
  mAxeWidth   = 0;  mAxeLength = 1.2;

  bRnrNames   = true;
  bRnrTiles   = true;  bRnrFrames = true;
  mNameOffset = 0.99;
  mTextCol.rgba(1, 1, 1);
  mTileCol.rgba(0, 0, 0.3);
  mTilePos = "";
}

/**************************************************************************/
