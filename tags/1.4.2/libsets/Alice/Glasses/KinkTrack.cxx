// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// KinkTrack
//
//

#include "KinkTrack.h"
#include "KinkTrack.c7"

ClassImp(KinkTrack)

/**************************************************************************/

void KinkTrack::_init()
{
  // *** Set all links to 0 ***
  mKink = 0;
}

/**************************************************************************/

KinkTrack:: KinkTrack(Kink* kink, const Text_t* n, const Text_t* t) :
  TrackBase(n,t) 
{
  _init();
  mKink = kink;
  SetName(GForm("KinkTrack %d",mKink->fLabel));
  mV = GForm("% 4.f, % 4.f, % 4.f",  mKink->fV[0],  mKink->fV[1], mKink->fV[2]);
  mP = GForm("% 6.3f, % 6.3f, % 6.3f", mKink->fP[0],  mKink->fP[1], mKink->fP[2]);
}

/**************************************************************************/
