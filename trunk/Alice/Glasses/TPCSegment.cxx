// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TPCDigit
//
//

#include "TPCSegment.h"
#include "TPCSegment.c7"

ClassImp(TPCSegment)

/**************************************************************************/

void TPCSegment::_init()
{
  // *** Set all links to 0 ***
  mSegment = 0;
  mDigInfo = 0;

  // Override from ZGlass
  bUseDispList = true;
}
/**************************************************************************/


/**************************************************************************/
