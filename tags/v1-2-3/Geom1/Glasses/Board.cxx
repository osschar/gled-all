// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Board.h"
//#include <Glasses/ZImage.h>

#include "FL/gl.h"
#include "GL/glu.h"

#include <stdio.h>

ClassImp(Board)

/**************************************************************************/

void Board::_init()
{
  mTexture = 0;
  mULen = mVLen = 1;
  bFillBack = false;
  mColor.rgba(1,0,0);
}

/**************************************************************************/

#include "Board.c7"
