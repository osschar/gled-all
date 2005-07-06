// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// RecData
//
//

#include "ESDParticle.h"

ClassImp(ESDParticle)

/**************************************************************************/

void ESDParticle::_init()
{
  bE = false; 
  fSign = 0;
  fLabel = -1;
  fStatus = 0;
  //fV = {0};
}

/**************************************************************************/
ESDParticle::ESDParticle( Double_t* v, Double_t* p, Int_t lab, Int_t sign)
{
  _init();
  fSign=sign;
  fLabel=lab;
  //vertex
  fV[0]= v[0];
  fV[1]= v[1];
  fV[2]= v[2];
  // momentum
  fP[0]= p[0];
  fP[1]= p[1];
  fP[2]= p[2];
}


/**************************************************************************/
