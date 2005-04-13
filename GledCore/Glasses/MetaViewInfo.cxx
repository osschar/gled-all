// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// MetaViewInfo
//
//

#include "MetaViewInfo.h"
#include "MetaViewInfo.c7"

ClassImp(MetaViewInfo)

/**************************************************************************/

void MetaViewInfo::_init()
{
  // *** Set all links to 0 ***
  mW = 32; mH = 8;
  bExpertP = false;
}

/**************************************************************************/

void MetaViewInfo::Size(int w, int h)
{
  mW = w; mH = h;
  Stamp(FID());
}

/**************************************************************************/
