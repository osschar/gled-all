// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// V0Track
//
//

#include "V0Track.h"
#include "V0Track.c7"

ClassImp(V0Track)

/**************************************************************************/

void V0Track::_init()
{
  // *** Set all links to 0 ***
}

/**************************************************************************/
V0Track:: V0Track(ESDParticle* esd, const Text_t* n, const Text_t* t):RecTrack(n,t) 
{
  _init();
  mESD = esd;
  mV = GForm("% 4.f, % 4.f, % 4.f",  mESD->fV[0],  mESD->fV[1], mESD->fV[2]);
  mP = GForm("% 6.3f, % 6.3f, % 6.3f", mESD->fP[0],  mESD->fP[1], mESD->fP[2]);
  /*
    tV0->mVM[0]=Float_t(esd->fPM[0]); 
    tV0->mVM[1]=Float_t(esd->fPM[1]); 
    tV0->mVM[2]=Float_t(esd->fPM[2]);
    tV0->SetName(GForm("V0 %d : %d {%d}", esd->fLabels[0], esd->fLabels[1], esd->fPDG));
  */

}

/**************************************************************************/
