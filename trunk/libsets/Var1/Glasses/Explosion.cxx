// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Explosion.h"
#include "Explosion.c7"

// Explosion

//______________________________________________________________________________
//
//

ClassImp(Explosion);

//==============================================================================

void Explosion::_init()
{
  mExplodeTime = 0;
  mExplodeDuration = 2;
}

Explosion::Explosion(const Text_t* n, const Text_t* t) :
  ZGlass(n, t)
{
  _init();
}

Explosion::~Explosion()
{}

//==============================================================================
