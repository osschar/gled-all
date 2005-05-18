// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_MCParticle_H
#define Alice_MCParticle_H

#include <TParticle.h>

class MCParticle : public TParticle {
 private:
  void _init();

 public:
  Int_t    fLabel;         // X{GS}   
  Int_t    fEvaID;      // X{GS}
  Bool_t   bDecayed;       // X{GS}
  Double_t fDx,  fDy, fDz;
  Double_t fDPx, fDPy,fDPz;
  Float_t  fDt;

  MCParticle(const Text_t* n="MCParticle", const Text_t* t=0) :
    TParticle()
  { _init(); }

  const TParticle& operator=(const TParticle& p)
  { *((TParticle*)this) = p; return *this; }

#include "MCParticle.h7"
  ClassDef(MCParticle, 1)
    }; // endclass MCParticle

#endif
