// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// RecTrackRS
//
//

#include "RecTrackRS.h"
#include "RecTrackRS.c7"

ClassImp(RecTrackRS)

/**************************************************************************/

  void RecTrackRS::_init()
{
  // *** Set all links to 0 ***
  bFitKinks = false;
  bFitV0 = false;
}

/**************************************************************************/


/**************************************************************************/
