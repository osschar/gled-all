// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// MCParticle
//
//

#include "MCParticle.h"

ClassImp(MCParticle);

/**************************************************************************/

void MCParticle::_init()
{
  bDecayed = false;
}
