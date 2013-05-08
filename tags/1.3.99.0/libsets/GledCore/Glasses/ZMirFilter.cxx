// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZMirFilter.h"
#include "ZMirFilter.c7"

//__________________________________________________________________________
// ZMirFilter
//
//

ClassImp(ZMirFilter);

/**************************************************************************/

void ZMirFilter::_init()
{
  mMapNoneTo = R_None;
}

/**************************************************************************/
