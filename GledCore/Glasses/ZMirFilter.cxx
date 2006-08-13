// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZMirFilter
//
//

#include "ZMirFilter.h"

#include "ZMirFilter.c7"

ClassImp(ZMirFilter)

/**************************************************************************/

void ZMirFilter::_init()
{
  mMapNoneTo = R_None;
}

/**************************************************************************/
