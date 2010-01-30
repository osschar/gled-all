// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// Hit
//
//

#include "Hit.h"

ClassImp(Hit)

/**************************************************************************/

Hit::Hit(UChar_t Detector, Int_t Particle, Int_t Eva, Float_t X, Float_t Y, Float_t Z ):TObject()
{ 
  fDetID=Detector;
  fLabel=Particle;
  fEvaLabel=Eva;
  x=X;y=Y;z=Z;
}

/**************************************************************************/

void Hit::Dump() 
{
  printf("XYZ(%f,%f,%f) eva(%d) label(%d) det(%d) \n",
	 x,y,z,fEvaLabel, fLabel, fDetID);
}

/**************************************************************************/
