// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_Amphitheatre_H
#define Geom1_Amphitheatre_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>

#include <TRandom.h>

class Amphitheatre : public ZNode {
  MAC_RNR_FRIENDS(Amphitheatre);

private:
  void _init();

protected:
  struct Chair {
    TVector3 fPos;
    ZNode*   fNode;

    Chair() : fPos(), fNode(0) {}
    Chair(TVector3& p, ZNode* n=0) : fPos(p), fNode(n) {}

    bool freep() { return fNode == 0; }
  };
  typedef list<Chair>           lChair_t;
  typedef list<Chair>::iterator lChair_i;

  ZList*		mNewGuests;  // X{gS} L{}
  lChair_t		mChairs;

  Int_t			mNumCh;      // X{gS} 7 ValOut(-join=>1)
  Int_t			mNumChFree;  // X{gS} 7 ValOut()

  Int_t			mStageSides; // X{gS} 7 Value(-range=>[0,100,1], -join=>1)
  Float_t		mStageRot;   // X{gS} 7 Value(-range=>[-1,1,1,1000])
  Float_t		mStageSize;  // X{gS} 7 Value(-range=>[0,10,1,1000], -join=>1)
  Float_t		mChairSize;  // X{gS} 7 Value(-range=>[0,10,1,1000])

  Float_t		mGuestSize;  // X{gS} 7 Value(-range=>[0,10,1,1000], -join=>1)
  Float_t	     mGuestScaleFac; // X{gS} 7 Value(-range=>[0,1,1,1000])

  Float_t		mRepX0;        // X{gS} 7 Value(-range=>[0,10,1,1000], -join=>1)
  Float_t		mRepXm;        // X{gS} 7 Value(-range=>[0,10,1,1000], -join=>1)
  Float_t		mRepXM;        // X{gS} 7 Value(-range=>[0,10,1,1000])
  Float_t		mGuestStep;    // X{gS} 7 Value(-range=>[0,10,1,1000], -join=>1)
  UInt_t		mStepSleepMS;  // X{gS} 7 Value(-range=>[0,1e6,1])
  Bool_t		bChairHunt;    // X{gS} 7 BoolOut()

  Chair* closest_free_chair(TVector3& pos);

  void   chair_hunt();            // X{E}
  void   chair_hunt_emit_mir();

  void   fix_guest_scale(ZNode* guest, bool finalp=false);

  //--------------------------------

  Bool_t		bRnrStage;  // X{gS}  7 Bool(-join=>1)
  Bool_t		bRnrChairs; // X{gS}  7 Bool()

  ZColor		mStageCol;  // X{gSP} 7 ColorButt(-join=>1)
  ZColor		mChairCol;  // X{gSP} 7 ColorButt()

  TRandom		mRnd;       //
  Double_t rnd(Double_t k=1, Double_t n=0);

public:
  Amphitheatre(const Text_t* n="Amphitheatre", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  virtual void AdEnlightenment();

  void CreateChairs(Float_t radius=4, Float_t xoffset=0, Float_t z=0,
		    Float_t dphi_deg=90, Int_t nchair=16); // X{E}
  void RemoveChairs();         // X{E} 7 MButt()

  void AddGuest(ZNode* guest); // X{E} C{1}
  void ClearFailedGuests();    // X{E} 7 MButt(-join=>1)
  void ClearAmphitheatre();    // X{E} 7 MButt()

  void StartHunt(); // X{E} 7 MButt(-join=>1)
  void StopHunt();  // X{E} 7 MButt()

  void MakeRandomGuests(Int_t nguests=10, Float_t box_size=10); // X{E} 7 MButt()

#include "Amphitheatre.h7"
  ClassDef(Amphitheatre, 1)
}; // endclass Amphitheatre

GlassIODef(Amphitheatre);

#endif
