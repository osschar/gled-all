// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ExtendioExplosion.h"
#include "Extendio.h"
#include "ExtendioExplosion.c7"

#include "Tringula.h"

// ExtendioExplosion

//______________________________________________________________________________
//
//

ClassImp(ExtendioExplosion);

//==============================================================================

void ExtendioExplosion::_init()
{
  // From ZGlass.
  bUseDispList  = true;
}

ExtendioExplosion::ExtendioExplosion(const Text_t* n, const Text_t* t) :
  Explosion(n, t)
{
  _init();
}

ExtendioExplosion::~ExtendioExplosion()
{}

//==============================================================================

void ExtendioExplosion::TimeTick(Double_t t, Double_t dt)
{
  mExplodeTime += dt;
  if (mExplodeTime > mExplodeDuration)
  {
    mExtendio->GetTringula()->ExplosionFinished(this);
  }
}
