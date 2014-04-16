// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Flyer_H
#define Var1_Flyer_H

#include <Glasses/Dynamico.h>

class Flyer : public Dynamico
{
  MAC_RNR_FRIENDS(Flyer);

private:
  void _init();

protected:
  Float_t  mHeight;       // X{GS} 7 ValOut()

  Float_t  mGravHChange;
  Bool_t   bGravFixUpDir; // X{GS} 7 Bool()

  Float_t  mTerrainSafety;      //! Safe distance from the terrain.
  Float_t  mTerrainProbeRadius; //! Radius of the last terrain probe.

public:
  Flyer(const Text_t* n="Flyer", const Text_t* t=0);
  virtual ~Flyer();

  virtual void TimeTick(Double_t t, Double_t dt);

#include "Flyer.h7"
  ClassDef(Flyer, 1);
}; // endclass Flyer

#endif
