// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Explosion_H
#define Var1_Explosion_H

#include <Glasses/ZGlass.h>
#include <Stones/TimeMakerClient.h>

class Explosion : public ZGlass,
		  public TimeMakerClient
{
  MAC_RNR_FRIENDS(Explosion);

private:
  void _init();

protected:
  Float_t         mExplodeTime;     // X{GS}
  Float_t         mExplodeDuration; // X{GS}

public:
  Explosion(const Text_t* n="Explosion", const Text_t* t=0);
  virtual ~Explosion();

  // Should really leave it undefined and make Explosion abstract.
  virtual void TimeTick(Double_t t, Double_t dt) {}

#include "Explosion.h7"
  ClassDef(Explosion, 1);
}; // endclass Explosion

#endif
