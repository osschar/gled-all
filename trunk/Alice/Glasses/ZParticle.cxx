// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZAliTrack
//
//

#include "ZParticle.h"
#include <Glasses/ZAliLoad.h>
#include "ZParticle.c7"
#include <Glasses/ZQueen.h>

ClassImp(ZParticle);

typedef list<ZParticle*>                   lpZParticle_t;
typedef list<ZParticle*>::iterator         lpZParticle_i;
/**************************************************************************/

void ZParticle::_init()
{
  // override from ZGlass 
  bUseDispList=true;

  mParticle = 0;
  mNDaughters = 0;
}

ZParticle::ZParticle(MCParticle* p, const Text_t* n, const Text_t* t):ZNode(n,t) 
{
  _init();
  mParticle = p;
  TDatabasePDG db; 
  TParticlePDG* pdgp = db.GetParticle(p->GetPdgCode());
  SetName(GForm("%d %s",p->GetLabel(), pdgp->GetName()));
  mNDaughters = p->GetNDaughters();
  mV = GForm("% 4.f, % 4.f, % 4.f", mParticle->Vx(), mParticle->Vy(), mParticle->Vz());
  if(mParticle->bDecayed) mVDecay = GForm("% 4.f, % 4.f, % 4.f", mParticle->fDx, mParticle->fDy, mParticle->fDz);
  mP = GForm("% 6.3f, % 6.3f, % 6.3f", mParticle->Px(), mParticle->Py(), mParticle->Pz());
}


void ZParticle::ImportDaughters(ZAliLoad* alil)
{
  static const string _eh("ZParticle::ImportDaughters");
  if (alil == 0) {
    alil = GrepParentByGlass<ZAliLoad*>();
    if(alil == 0) throw(_eh + "can't set ZAliLoad.");
  }
  Int_t d0 =  mParticle->GetDaughter(0), d1 = mParticle->GetDaughter(1);
  if ( mParticle->GetNDaughters() == 0 ) return;
  for(int i=d0; i<=d1; ++i) {
    MCParticle* tp = alil->Particle(i);
    ZParticle* p = new ZParticle(tp, GForm("%d %s", i, tp->GetName())); 
    mQueen->CheckIn(p);Add(p);
  }
  mStampReqTring = Stamp(FID());
}

void ZParticle::ImportDaughtersRec(ZAliLoad* alil)
{
  static const string _eh("ZParticle::ImportDaughters");
  if (alil == 0) {
    alil = GrepParentByGlass<ZAliLoad*>();
    if(alil == 0) throw(_eh + "can't set ZAliLoad.");
  }

  Int_t d0 =  mParticle->GetDaughter(0), d1 = mParticle->GetDaughter(1);
  if ( mParticle->GetNDaughters() == 0 ) return;
  for(int i=d0; i<=d1; ++i) {
    // printf("%s ImportDaughtersRec :: new particle idx %d \n",GetName(), i);
    MCParticle* tp = alil->Particle(i);
    ZParticle* p = new ZParticle(tp, GForm("%d %s", i, tp->GetName())); 
    mQueen->CheckIn(p);Add(p);
    p->ImportDaughtersRec(alil);
  }
  mStampReqTring = Stamp(FID());
}

/**************************************************************************/

void  ZParticle::ImportHits(ZAliLoad* alil)
{
  static const string _eh("ZParticle::ImportHits ");
  if (alil == 0) {
    alil = GrepParentByGlass<ZAliLoad*>();
    if(alil == 0) throw(_eh + "can't set ZAliLoad.");
  }
  char selection[128];
  sprintf (selection, "particle_ID==%d", mParticle->fLabel);
  alil->SelectHits(0, selection);
}

void  ZParticle::ImportHitsFromPrimary(ZAliLoad* alil)
{
  static const string _eh("ZParticle::ImportHits ");
  if (alil == 0) {
    alil = GrepParentByGlass<ZAliLoad*>();
    if(alil == 0) throw(_eh + "can't set ZAliLoad.");
  }
  char selection[128];
  sprintf (selection, "eva_ID==%d", mParticle->fLabel);
  alil->SelectHits(0, selection);
}

/**************************************************************************/

void ZParticle::Dump()
{
  printf("ZParticle %s, v(%3.f, %3.f, %3.f), p(%6.5f, %6.5f, %6.5f), m(%.3f)\n", 
         GetName(),
	 mParticle->Vx(), mParticle->Vy(), mParticle->Vz(),
         mParticle->Px(), mParticle->Py(), mParticle->Pz(),
         mParticle->GetMass());

  
  printf("ZParticle %s, decay  v(%3.f, %3.f, %3.f), p(%6.4f, %6.4f, %6.4f), t(%f) \n",
	 GetName(),
	 mParticle->fDx,  mParticle->fDy,  mParticle->fDz,
	 mParticle->fDPx, mParticle->fDPy, mParticle->fDPz,
	 mParticle->fDt);
 
}





