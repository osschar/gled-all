// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Explosion.h"
#include "Explosion.c7"
#include "Tringula.h"

// Explosion

//______________________________________________________________________________
//
//

ClassImp(Explosion);

//==============================================================================

void Explosion::_init()
{
  mTringula = 0;

  mExplodeTime = 0;
  mExplodeDuration = 1;
}

Explosion::Explosion(const Text_t* n, const Text_t* t) :
  ZGlass(n, t)
{
  _init();
}

Explosion::~Explosion()
{}

//==============================================================================

void Explosion::SetTringula(Tringula* tring)
{
  // Set tringula to which the explosion is attached.
  // Sub-classes override this to reinitialize cached data.

  mTringula = tring;
}

//==============================================================================

void Explosion::TimeTick(Double_t t, Double_t dt)
{
  mExplodeTime += dt;
  if (mExplodeTime > mExplodeDuration)
  {
    mTringula->ExplosionFinished(this);
  }
}
