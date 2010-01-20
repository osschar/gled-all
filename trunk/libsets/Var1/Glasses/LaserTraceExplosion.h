// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_LaserTraceExplosion_H
#define Var1_LaserTraceExplosion_H

#include <Glasses/Explosion.h>
#include <Stones/HTrans.h>

class LaserTraceExplosion : public Explosion
{
  MAC_RNR_FRIENDS(LaserTraceExplosion);

protected:
  HPointF      mA; // X{r}
  HPointF      mB; // X{r}

public:
  LaserTraceExplosion(const Text_t* n="LaserTraceExplosion", const Text_t* t=0);
  virtual ~LaserTraceExplosion();

#include "LaserTraceExplosion.h7"
  ClassDef(LaserTraceExplosion, 1);
}; // endclass LaserTraceExplosion

#endif
