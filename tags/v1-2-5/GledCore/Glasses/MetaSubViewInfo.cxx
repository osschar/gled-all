// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// MetaSubViewInfo
//
//

#include "MetaSubViewInfo.h"
#include "MetaSubViewInfo.c7"

ClassImp(MetaSubViewInfo)

/**************************************************************************/

void MetaSubViewInfo::_init()
{
  // *** Set all links to 0 ***
  mX = mY = 0;
}

/**************************************************************************/

void MetaSubViewInfo::Position(int x, int y)
{
  mX = x; mY = y;
  Stamp(FID());
}

/**************************************************************************/
