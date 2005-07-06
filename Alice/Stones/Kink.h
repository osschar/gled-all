// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_Kink_H
#define Alice_Kink_H

#include <Stones/ESDParticle.h>

class Kink : public ESDParticle {

private:
  void _init();

protected:

public:
  Double_t fEV[3];   // end point
  Int_t    fDLabel;  // daughter label  
  Double_t fDP[3];   // daughter momentum

  Kink(const Text_t* n="Kink", const Text_t* t=0) :
    ESDParticle(n,t)
  { _init(); }


#include "Kink.h7"
  ClassDef(Kink, 1)
}; // endclass Kink

#endif
