// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_RelParticle_H
#define Geom1_RelParticle_H

#include <Glasses/ZGlass.h>
#include <TLorentzVector.h>

class RelParticle : public ZGlass {
  MAC_RNR_FRIENDS(RelParticle);

private:
  void _init();

protected:
  TLorentzVector	mX;	// X{GSR} 7 LorentzVector()
  TLorentzVector	mP;	// X{GSR} 7 LorentzVector()

public:
  RelParticle(const Text_t* n="RelParticle", const Text_t* t=0) : ZGlass(n,t) { _init(); }


#include "RelParticle.h7"
  ClassDef(RelParticle, 1)
}; // endclass RelParticle


#endif
