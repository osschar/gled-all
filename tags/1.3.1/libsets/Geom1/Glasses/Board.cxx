// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Board.h"
#include <Glasses/ZImage.h>
#include "Board.c7"

ClassImp(Board);

/**************************************************************************/

void Board::_init()
{
  mTexX0 = mTexY0 = 0;
  mTexX1 = mTexY1 = 1;

  mULen  = mVLen  = 1;
  mUDivs = mVDivs = 0;
}

/**************************************************************************/
