// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AlListener
//
//

#include "AlListener.h"
#include "AlListener.c7"

#include <AL/al.h>

ClassImp(AlListener);

/**************************************************************************/

void AlListener::_init()
{
  // *** Set all links to 0 ***
  mLocationType = LT_Camera;
  mGain = 1;
}

/**************************************************************************/

void AlListener::EmitSourceRay()
{
  alListenerf(AL_GAIN, mGain);
}
