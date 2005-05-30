// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// RecTrack
//
//

#include "RecTrack.h"
#include "RecTrack.c7"

ClassImp(RecTrack)

/**************************************************************************/

void RecTrack::_init()
{
  // *** Set all links to 0 ***
  mESD=0;
}

/**************************************************************************/

RecTrack:: RecTrack(ESDParticle* esd, const Text_t* n, const Text_t* t):ZNode(n,t) 
{
  _init();
  mESD = esd;
  SetName(GForm("RecTrack %d",mESD->fLabel));
  mV = GForm("% 4.f, % 4.f, % 4.f",  mESD->fV[0],  mESD->fV[1], mESD->fV[2]);
  mP = GForm("% 6.3f, % 6.3f, % 6.3f", mESD->fP[0],  mESD->fP[1], mESD->fP[2]);

}

void RecTrack::Dump()
{
  printf("RecTrack %d, V(%f,%f,%F), P(%f,%f,%F) \n",mESD->fLabel, 
	 mESD->fV[0], mESD->fV[1], mESD->fV[2], 
	 mESD->fP[0], mESD->fP[1], mESD->fP[2]);
}
