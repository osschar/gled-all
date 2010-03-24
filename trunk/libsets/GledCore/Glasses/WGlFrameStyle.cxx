// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// WGlFrameStyle
//
//

#include "WGlFrameStyle.h"
#include "WGlFrameStyle.c7"

ClassImp(WGlFrameStyle);

/**************************************************************************/

void WGlFrameStyle::_init()
{
  mHAlign     = HA_Left;
  mVAlign     = VA_Center;
  mDefDx      = 2;
  mTextFadeW  = 0.95;
  mTextYSize  = 0.4;
  mFrameW     = 0;

  bDrawTile  = true;
  bMinTile   = false;
  bDrawFrame = true;
  mXBorder   = mYBorder   = 0.1;

  mTextColor  .rgba(1, 1, 1);
  mFrameColor .rgba(1, 1, 1);
  mTileColor  .rgba(0.3, 0.3, 0.5, 0.6);
  mBelowMColor.rgba(0.5, 0.5, 0.7, 0.8);
}

/**************************************************************************/

void WGlFrameStyle::StandardPersp()
{
  SetTextYSize(0.6);
  SetDefDx(2);
  SetTextFadeW(1.5);
  SetXBorder(0.1); SetYBorder(0.1);
}

void WGlFrameStyle::StandardFixed()
{
  SetTextYSize(0.4);
  SetDefDx(1.7);
  SetTextFadeW(1.5);
  SetXBorder(0.1); SetYBorder(0.1);
}

void WGlFrameStyle::StandardPixel()
{
  SetTextYSize(18);
  SetDefDx(90);
  SetTextFadeW(1.5);
  SetXBorder(5); SetYBorder(2);
}

/**************************************************************************/
