// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// Amphitheatre
//
//

#include "Amphitheatre.h"
#include "Amphitheatre.c7"
#include <Glasses/ZQueen.h>
#include <Glasses/Sphere.h>

ClassImp(Amphitheatre)

/**************************************************************************/

void Amphitheatre::_init()
{
  mNewGuests = 0;

  mNumCh = mNumChFree = 0; 

  mStageSides = 7; mStageRot  = 0;
  mStageSize  = 1; mChairSize = 0.15;

  mGuestSize     = 0.2;
  mGuestScaleFac = 0.01;

  mRepX0 = 2*mChairSize;
  mRepXm = 0.5*mGuestSize;
  mRepXM = 5*mGuestSize;

  mGuestStep    = 0.08;
  mStepSleepMS  = 100;
  bChairHunt    = false;

  bRnrStage  = true; bRnrChairs = true;
  // Stage col white.
  mChairCol.rgba(0.6, 0.9, 0.6);

  mRnd.SetSeed(0); // !! This NOT distributed.
}

/**************************************************************************/

void Amphitheatre::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  if(mNewGuests == 0) {
    ZList* l = new ZList("GuestList", GForm("Newly arrived guests of %s", GetName()));
    l->SetElementFID(ZNode::FID());
    l->SetMIRActive(false);
    mQueen->CheckIn(l);
    SetNewGuests(l);
  }
}

/**************************************************************************/

void Amphitheatre::CreateChairs(Float_t radius, Float_t xoffset, Float_t z,
				Float_t dphi_deg, Int_t nchair)
{
  Float_t dphi = dphi_deg * TMath::DegToRad();
  Float_t step = dphi / (nchair - 1);
  Float_t phi0 = -dphi / 2;
  for(int i=0; i<nchair; ++i) {
    Float_t p = phi0 + i*step;
    TVector3 pos = TVector3(radius*TMath::Cos(p), radius*TMath::Sin(p), z);
    pos[0] += xoffset;
    mChairs.push_back(Chair(pos));
  }
  mNumCh += nchair; mNumChFree += nchair; 
  Stamp(LibID(), ClassID());
}


void Amphitheatre::RemoveChairs()
{
  mChairs.clear();
  mNumCh = mNumChFree = 0;
  Stamp(LibID(), ClassID());
}

/**************************************************************************/

void Amphitheatre::AddGuest(ZNode* guest)
{
  mNewGuests->Add(guest);
  guest->WriteLock();
  ZNode* gp = guest->GetParent();
  if(gp != this) {
    if(gp != 0) {
      auto_ptr<ZTrans> t( ZNode::BtoA(this, guest) );
      if(t.get() != 0) {
	Float_t s[3];
	t->Unscale3(s[0], s[1], s[2]);
	guest->SetTrans(*t);
	guest->SetScales(s[0], s[1], s[2]);
      }
    }
    guest->SetParent(this);
    Add(guest);
  }
  guest->WriteUnlock();
}

void Amphitheatre::ClearFailedGuests()
{
  lpZGlass_i i, end;
  mNewGuests->WriteLock();
  mNewGuests->BeginIteration(i, end);
  while(i != end) { Remove(*i); ++i; }
  mNewGuests->EndIteration();
  mNewGuests->ClearList();
  mNewGuests->WriteUnlock();
}

void Amphitheatre::ClearAmphitheatre()
{
  ClearFailedGuests();
  ClearList();
  for(lChair_i c=mChairs.begin(); c!=mChairs.end(); ++c) c->fNode = 0;
  mNumChFree = mNumCh;
  Stamp(LibID(), ClassID());
}

/**************************************************************************/
/**************************************************************************/

Amphitheatre::Chair* Amphitheatre::closest_free_chair(TVector3& pos)
{
  if(mNumChFree <= 0)
    return 0;

  Double_t min_r2 = 1e9;
  lChair_i min_ch;
  for(lChair_i i = mChairs.begin(); i != mChairs.end(); ++i) {
    if(i->freep()) {
      TVector3 dr(i->fPos - pos);
      Double_t r2 = dr.Mag2();
      if(r2 < min_r2) {
	min_r2  = r2;
	min_ch = i;
      }
    }
  }
  return &(*min_ch);
}

/**************************************************************************/

void Amphitheatre::chair_hunt()
{
  Bool_t changep = false;

  typedef list<ZNode*>		 lpZNode_t;
  typedef list<ZNode*>::iterator lpZNode_i;

  lpZNode_t nodes;
  mNewGuests->CopyByGlass<ZNode*>(nodes);
  
  if(bChairHunt == false || nodes.empty() || mNumChFree <= 0) {
    StopHunt();
    return;
  }

  for(lpZNode_i n=nodes.begin(); n!=nodes.end(); ++n) {
    ZNode* node = *n;
    if(mNumChFree <= 0)
      break;

    TVector3 x   = node->RefTrans().GetPosVec3();
    Chair* chair = closest_free_chair(x);
    
    x = chair->fPos - x;
    Double_t dx = x.Mag();
    if(dx > mGuestStep) {
      x *= (mGuestStep/dx);
    }
    // printf("Got r = %f, %f, %f; %f\n", f.x(), f.y(), f.z(), f.Mag());

    bool finalp = (dx <= mGuestStep); // Guest has reached the chair.

    node->WriteLock();
    node->Move3(x.x(), x.y(), x.z());
    fix_guest_scale(node, finalp);
    // !! Could rotate it along travel axis.
    node->WriteUnlock();

    if(finalp) {
      changep = true;
      // !!! Should also rotate it towards the stage.
      mNewGuests->WriteLock();
      mNewGuests->Remove(node);
      mNewGuests->WriteUnlock();
      chair->fNode = node;
      --mNumChFree;
      lpZNode_i to_remove = n--;
      nodes.erase(to_remove);
    }
  }

  // Spread the remaining roamers if too close.
  const Double_t rxm = mRepX0*mRepXm;
  const Double_t rxM = mRepX0*mRepXM;

  for(lpZNode_i n=nodes.begin(); n!=nodes.end(); ++n) {
    TVector3 f;
    ZNode* node = *n;    
    TVector3 x(node->RefTrans().GetPosVec3());
    // loop over rivals
    for(lpZNode_i o=nodes.begin(); o!=nodes.end(); ++o) {
      if(o == n) continue;
      TVector3 y = (*o)->RefTrans().GetPosVec3();
      y -= x;
      Double_t d = y.Mag();
      if(d < rxM) {
	if(d < rxm) { y *= rxm/d; d = rxm; }
	y *= mRepX0*(1/d - 1/rxm)/d;
	f += y;
      }
    }
    // loop over taken chairs
    for(lChair_i i=mChairs.begin(); i != mChairs.end(); ++i) {
      if(! i->freep()) {
	TVector3 y = i->fNode->RefTrans().GetPosVec3();
	y -= x;
	Double_t d = y.Mag();
	if(d < rxM) {
	  if(d < rxm) { y *= rxm/d; d = rxm; }
	  y *= mRepX0*(1/d - 1/rxm)/d;
	  f += y;
	}
      }
    }

    Double_t fmag = f.Mag();
    if(fmag != 0) {
      if(fmag > mGuestStep/2) f *= mGuestStep/2/fmag;
      node->WriteLock();
      node->Move3(f.x(), f.y(), f.z());
      node->WriteUnlock();
    }
  }

  if(changep)
    Stamp(LibID(), ClassID());

  chair_hunt_emit_mir();
}

void Amphitheatre::chair_hunt_emit_mir()
{
  GTime at(GTime::I_Now); at += 1000l*mStepSleepMS;
  auto_ptr<ZMIR> mir( S_chair_hunt() );
  mSaturn->DelayedShootMIR(mir, at);
}

/**************************************************************************/

void Amphitheatre::fix_guest_scale(ZNode* guest, bool finalp)
{
  Float_t s[3];
  s[0] = guest->GetSx(); s[1] = guest->GetSy(); s[2] = guest->GetSz();
  Float_t avgs = (s[0]+s[1]+s[2])/3;
  Float_t ds = mGuestSize - avgs;
  // printf("%-10s %8f %8f\n", guest->GetName(), avgs, ds);
  if(TMath::Abs(ds) > 0.05 || finalp) {
    if(!finalp) ds *= mGuestScaleFac; // exponential approach
    guest->SetSx(s[0]*(1 + ds/s[0]));
    guest->SetSy(s[1]*(1 + ds/s[1]));
    guest->SetSz(s[2]*(1 + ds/s[2]));
  }
}

/**************************************************************************/

void Amphitheatre::StartHunt()
{
  if(bChairHunt) return;
  bChairHunt = true;
  chair_hunt_emit_mir();
  Stamp(LibID(), ClassID());
}

void Amphitheatre::StopHunt()
{
  if(!bChairHunt) return;
  bChairHunt = false;
  Stamp(LibID(), ClassID());
}

/**************************************************************************/

inline
Double_t Amphitheatre::rnd(Double_t k, Double_t n)
{
  return k*mRnd.Rndm() + n;
}

void Amphitheatre::MakeRandomGuests(Int_t nguests, Float_t box_size)
{
  // This is for testing only; NOT DISTRIBUTED.

  Float_t bh = box_size / 2;
  for(Int_t i=0; i<nguests; ++i) {
    Sphere* s = new Sphere(GForm("Guest %d", int(1000*mRnd.Rndm())));

    s->Set3Pos(rnd(box_size, -bh), rnd(box_size, -bh), rnd(box_size, -bh));
    s->SetColor(rnd(0.6,0.4), rnd(0.6,0.4), rnd(0.6,0.4));
    s->SetLOD(12);
    s->SetRadius(0.1);
    mQueen->CheckIn(s);
    Add(s);
    mNewGuests->WriteLock();
    mNewGuests->Add(s);
    mNewGuests->WriteUnlock();
  }
}
