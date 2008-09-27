// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// MCTrack
//
//

#include "MCTrack.h"
#include <Glasses/VSDSelector.h>
#include "MCTrack.c7"
#include <Glasses/ZQueen.h>

ClassImp(MCTrack);

typedef list<MCTrack*>                   lpMCTrack_t;
typedef list<MCTrack*>::iterator         lpMCTrack_i;

/**************************************************************************/

void MCTrack::_init()
{
  mParticle   = 0;
  mNDaughters = 0;
}

MCTrack::MCTrack(MCParticle* p, const Text_t* n, const Text_t* t) :
  TrackBase(n,t) 
{
  _init();
  // TParticlePDG* pdgp = TDatabasePDG::Instance()->GetParticle(p->GetPdgCode());
  TParticlePDG* pdgp = p->GetPDG();
  if(pdgp == 0) {
    p->ResetPdgCode(); pdgp = p->GetPDG();
  }
  mParticle = p;
  SetName(GForm("%s %d", pdgp->GetName(),p->GetLabel()));

  mNDaughters = p->GetNDaughters();
  if(mParticle->bDecayed) mVDecay = GForm("% 4.f, % 4.f, % 4.f", mParticle->fDx, mParticle->fDy, mParticle->fDz);
  mV = GForm("% 4.f, % 4.f, % 4.f", mParticle->Vx(), mParticle->Vy(), mParticle->Vz());
  mP = GForm("% 6.3f, % 6.3f, % 6.3f", mParticle->Px(), mParticle->Py(), mParticle->Pz());
}

/**************************************************************************/

void MCTrack::ImportDaughters(VSDSelector* sel)
{
  static const Exc_t _eh("MCTrack::ImportDaughters");
  if (sel == 0) {
    sel = GrepParentByGlass<VSDSelector>();
    if(sel == 0) throw(_eh + "can't set VSDSelector.");
  }
  Int_t d0 =  mParticle->GetDaughter(0), d1 = mParticle->GetDaughter(1);
  if ( mParticle->GetNDaughters() == 0 ) return;
  for(int i=d0; i<=d1; ++i) {
    MCParticle* tp = sel->Particle(i);
    MCTrack* p = new MCTrack(tp, GForm("%d %s", i, tp->GetName())); 
    mQueen->CheckIn(p);Add(p);
  }
  mStampReqTring = Stamp(FID());
}

void MCTrack::ImportDaughtersRec(VSDSelector* sel)
{
  static const Exc_t _eh("MCTrack::ImportDaughters");
  if (sel == 0) {
    sel = GrepParentByGlass<VSDSelector>();
    if(sel == 0) throw(_eh + "can't set VSDSelector.");
  }

  Int_t d0 =  mParticle->GetDaughter(0), d1 = mParticle->GetDaughter(1);
  if ( mParticle->GetNDaughters() == 0 ) return;
  for(int i=d0; i<=d1; ++i) {
    // printf("%s ImportDaughtersRec :: new particle idx %d \n",GetName(), i);
    MCParticle* tp = sel->Particle(i);
    MCTrack* p = new MCTrack(tp, GForm("%d %s", i, tp->GetName())); 
    mQueen->CheckIn(p);Add(p);
    p->ImportDaughtersRec(sel);
  }
  mStampReqTring = Stamp(FID());
}

/**************************************************************************/

void  MCTrack::ImportHits(VSDSelector* sel, Bool_t from_primary)
{
  static const Exc_t _eh("MCTrack::ImportHits ");
  if (sel == 0) {
    sel = GrepParentByGlass<VSDSelector>();
    if(sel == 0) throw(_eh + "can't set VSDSelector.");
  }
  char selection[128];
  if(from_primary){
    sprintf (selection, "fEvaLabel==%d", mParticle->fLabel);
  } else {
    sprintf (selection, "fLabel==%d", mParticle->fLabel);
  }
  sel->SelectHits(this, selection);
}

/**************************************************************************/

void  MCTrack::ImportClusters(VSDSelector* sel, Bool_t from_primary)
{
  static const Exc_t _eh("MCTrack::ImportClusters ");
  if (sel == 0) {
    sel = GrepParentByGlass<VSDSelector>();
    if(sel == 0) throw(_eh + "can't set VSDSelector.");
  }
  char selection[128];
  if(from_primary){
    sprintf (selection, "fEvaLabel==%d", mParticle->fLabel);
  } else {
    sprintf (selection, "fLabel==%d", mParticle->fLabel);
  }
  sel->SelectClusters(this, selection);
}

/**************************************************************************/

void MCTrack::SetDecayFromDaughter()
{
  MCTrack* last_d = dynamic_cast<MCTrack*>(BackElement());
  if(last_d) {
    mParticle->SetDecayed(true);
    mParticle->fDx = last_d->mParticle->Vx();
    mParticle->fDy = last_d->mParticle->Vy();
    mParticle->fDz = last_d->mParticle->Vz();
  } else {
    mParticle->SetDecayed(false);
  }
  mStampReqTring = Stamp(FID());
}

void MCTrack::ClearDecay()
{
  mParticle->SetDecayed(false);
  mStampReqTring = Stamp(FID());
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





