// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZAliTrack
//
//

#include "MCTrack.h"
#include <Glasses/ZAliLoad.h>
#include "MCTrack.c7"
#include <Glasses/ZQueen.h>

ClassImp(MCTrack);

typedef list<MCTrack*>                   lpMCTrack_t;
typedef list<MCTrack*>::iterator         lpMCTrack_i;
/**************************************************************************/

void MCTrack::_init()
{
  // override from ZGlass 
  bUseDispList=true;
  mParticle = 0;
  mNDaughters = 0;
}

MCTrack::MCTrack(MCParticle* p, const Text_t* n, const Text_t* t):TrackBase(n,t) 
{
  _init();
  mParticle = p;
  TParticlePDG* pdgp = TDatabasePDG::Instance()->GetParticle(p->GetPdgCode());
  if(pdgp == 0) {
    printf("Error PDG %d not found \n",p->GetPdgCode() );
  }else {
    SetName(GForm("%s %d", pdgp->GetName(),p->GetLabel()));
  }
  mNDaughters = p->GetNDaughters();
  if(mParticle->bDecayed) mVDecay = GForm("% 4.f, % 4.f, % 4.f", mParticle->fDx, mParticle->fDy, mParticle->fDz);
  mV = GForm("% 4.f, % 4.f, % 4.f", mParticle->Vx(), mParticle->Vy(), mParticle->Vz());
  mP = GForm("% 6.3f, % 6.3f, % 6.3f", mParticle->Px(), mParticle->Py(), mParticle->Pz());
}

/**************************************************************************/

void MCTrack::ImportDaughters(ZAliLoad* alil)
{
  static const string _eh("MCTrack::ImportDaughters");
  if (alil == 0) {
    alil = GrepParentByGlass<ZAliLoad*>();
    if(alil == 0) throw(_eh + "can't set ZAliLoad.");
  }
  Int_t d0 =  mParticle->GetDaughter(0), d1 = mParticle->GetDaughter(1);
  if ( mParticle->GetNDaughters() == 0 ) return;
  for(int i=d0; i<=d1; ++i) {
    MCParticle* tp = alil->Particle(i);
    MCTrack* p = new MCTrack(tp, GForm("%d %s", i, tp->GetName())); 
    mQueen->CheckIn(p);Add(p);
  }
  mStampReqTring = Stamp(FID());
}

void MCTrack::ImportDaughtersRec(ZAliLoad* alil)
{
  static const string _eh("MCTrack::ImportDaughters");
  if (alil == 0) {
    alil = GrepParentByGlass<ZAliLoad*>();
    if(alil == 0) throw(_eh + "can't set ZAliLoad.");
  }

  Int_t d0 =  mParticle->GetDaughter(0), d1 = mParticle->GetDaughter(1);
  if ( mParticle->GetNDaughters() == 0 ) return;
  for(int i=d0; i<=d1; ++i) {
    // printf("%s ImportDaughtersRec :: new particle idx %d \n",GetName(), i);
    MCParticle* tp = alil->Particle(i);
    MCTrack* p = new MCTrack(tp, GForm("%d %s", i, tp->GetName())); 
    mQueen->CheckIn(p);Add(p);
    p->ImportDaughtersRec(alil);
  }
  mStampReqTring = Stamp(FID());
}

/**************************************************************************/
void  MCTrack::ImportHits(ZAliLoad* alil, Bool_t from_primary)
{
  static const string _eh("MCTrack::ImportHits ");
  if (alil == 0) {
    alil = GrepParentByGlass<ZAliLoad*>();
    if(alil == 0) throw(_eh + "can't set ZAliLoad.");
  }
  char selection[128];
  if(from_primary){
    sprintf (selection, "fEvaLabel==%d", mParticle->fLabel);
  } else {
    sprintf (selection, "fLabel==%d", mParticle->fLabel);
  }
  alil->SelectHits(this, selection);
}

/**************************************************************************/
void  MCTrack::ImportClusters(ZAliLoad* alil, Bool_t from_primary)
{
  static const string _eh("MCTrack::ImportClusters ");
  if (alil == 0) {
    alil = GrepParentByGlass<ZAliLoad*>();
    if(alil == 0) throw(_eh + "can't set ZAliLoad.");
  }
  char selection[128];
  if(from_primary){
    sprintf (selection, "fEvaLabel==%d", mParticle->fLabel);
  } else {
    sprintf (selection, "fLabel==%d", mParticle->fLabel);
  }
  alil->SelectClusters(this, selection);
}
/**************************************************************************/

void MCTrack::Dump()
{
  printf("MCTrack %s, v(%3.f, %3.f, %3.f), p(%6.5f, %6.5f, %6.5f), m(%.3f)\n", 
         GetName(),
	 mParticle->Vx(), mParticle->Vy(), mParticle->Vz(),
         mParticle->Px(), mParticle->Py(), mParticle->Pz(),
         mParticle->GetMass());

  
  printf("MCTrack %s, decay  v(%3.f, %3.f, %3.f), p(%6.4f, %6.4f, %6.4f), t(%f) \n",
	 GetName(),
	 mParticle->fDx,  mParticle->fDy,  mParticle->fDz,
	 mParticle->fDPx, mParticle->fDPy, mParticle->fDPz,
	 mParticle->fDt);
 
}





