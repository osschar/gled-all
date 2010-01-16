// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_ExtendioExplosion_H
#define Var1_ExtendioExplosion_H

#include <Glasses/Explosion.h>

class Extendio;

class ExtendioExplosion : public Explosion
{
  MAC_RNR_FRIENDS(ExtendioExplosion);

private:
  void _init();

protected:
  // Does this have to be a link?
  ZLink<Extendio>    mExtendio; // X{GS} L{}

public:
  ExtendioExplosion(const Text_t* n="ExtendioExplosion", const Text_t* t=0);
  virtual ~ExtendioExplosion();

  virtual void TimeTick(Double_t t, Double_t dt);

#include "ExtendioExplosion.h7"
  ClassDef(ExtendioExplosion, 1);
}; // endclass ExtendioExplosion

#endif
