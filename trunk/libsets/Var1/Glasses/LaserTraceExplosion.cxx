// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "LaserTraceExplosion.h"
#include "LaserTraceExplosion.c7"

// LaserTraceExplosion

//______________________________________________________________________________
//
//

ClassImp(LaserTraceExplosion);

//==============================================================================

LaserTraceExplosion::LaserTraceExplosion(const Text_t* n, const Text_t* t) :
  Explosion(n, t)
{}

LaserTraceExplosion::~LaserTraceExplosion()
{}
