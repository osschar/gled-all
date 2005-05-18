// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// V0
//
//
#include <Glasses/ZQueen.h>
#include "V0.h"
#include "V0.c7"

#include <TParticlePDG.h>
#include <TDatabasePDG.h>


ClassImp(V0)

/**************************************************************************/
void V0::_init()
{
  // *** Set all links to 0 ***
  mPChild = 0;
  mNChild = 0;
  mESD->fSign = 0;
}

/**************************************************************************/
V0::V0(AliESDv0* av, const Text_t* n, const Text_t* t):RecTrack(n,t)
{
  _init();
  /*
    TDatabasePDG db; 
    TParticlePDG* pdgp = db.GetParticle(av->GetPdgCode());
    SetName(GForm("ESDv0 %s", pdgp->GetName()));
    av->GetNPxPyPz(mP[0], mP[1], mP[2]);
    av->GetXYZ(mV[0], mV[1], mV[2]);

    // P particle
    mPChild = new  RecTrack();
    av->GetPPxPyPz(mPChild->mP[0], mPChild->mP[1], mPChild->mP[2]);
    av->GetXYZ(mPChild->mV[0], mPChild->mV[1], mPChild->mV[2]);
    mPChild->SetName(GForm("%d P",av->GetPindex()));
    mQueen->CheckIn(mPChild);
    Add(mPChild);

    // N particle
    mNChild = new  RecTrack();
    av->GetNPxPyPz(mNChild->mP[0], mNChild->mP[1], mNChild->mP[2]);
    av->GetXYZ(mNChild->mV[0], mNChild->mV[1], mNChild->mV[2]);
    mNChild->SetName(GForm("%d P",av->GetNindex()));
    mNChild->mSign = -1;
    mQueen->CheckIn(mNChild);
    Add(mNChild);

    delete pdgp;
  */
}

/**************************************************************************/
