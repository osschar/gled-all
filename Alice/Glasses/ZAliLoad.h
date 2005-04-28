// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_ZAliLoad_H
#define Alice_ZAliLoad_H

#include <Glasses/ZNode.h>
#include <Glasses/ZParticle.h>
#include <Glasses/HitContainer.h>
#include <Stones/Hit.h>
#include <Glasses/TPCSegment.h>
#include <Stones/TPCDigitsInfo.h>
#include <Stones/MCParticle.h>

#include <TEventList.h>
#include <AliRunLoader.h>
#include <AliStack.h>
#include <AliMagF.h>
#include <AliTrackReference.h>

class ZAliLoad : public ZNode {
  MAC_RNR_FRIENDS(ZAliLoad);

private:
  void _init();
  TTree*                         mTreeK;   // X{g}
  TTree*                         mTreeH;   // X{g}
  TTree*                         mTreeTR;  // X{g}
  Hit                            mH, *mpH;      // needed for selection in mTreeH
  MCParticle                     mP, *mpP;      // needed for selection in mTreeK
  TPCDigitsInfo*                 mTPCDigInfo;

protected:
  ZParticle* get_track(Int_t tid);

  TString                  mDataDir;   // X{GS} 7 Filor(-dir=>1)
  Int_t			   mEvent;     // X{GS} 7 Value(-range=>[0,100,1])

  TString	           mVSDName;  // X{GS} 7 Textor()

  TFile*                   mFile;      // X{G} 
  TDirectory*              mDirectory; // X{G}

public:
  ZAliLoad(const Text_t* n="ZAliLoad", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  void SetupDataSource(Bool_t use_aliroot=false); // X{Ed} 7 MCWButt()

  void SetupEvent(Bool_t use_aliroot);

  void Write();      // X{Ed} 7 MButt()

  void ResetEvent(); // X{Ed} 7 MButt()

  // --------------------------------------------------------------
  // Kinematics

  void LoadKinematics();

  MCParticle* Particle(Int_t i);
  void PrintTreeK();

protected:
  TString    mParticleSelection;               // X{GS} 7 Textor()
public:
  void SelectParticles(ZNode* holder=0, const Text_t* selection=0,
		       Bool_t import_daughters=false
		       ); // X{Ed} C{1} 7 MCWButt()

  // --------------------------------------------------------------
  // Hits 

  void LoadHits();

protected:
  TString    mHitSelection;               // X{GS} 7 Textor()
public:
  void SelectHits(HitContainer* holder=0, const Text_t* selection=0
		  );     // X{Ed} C{1} 7 MCWButt()


  // --------------------------------------------------------------
  // TPC digits

  TPCSegment* ShowTPCSegment(Int_t segment_id, ZNode* holder = 0);  
  void        ShowTPCPlate(Int_t side = -1);   // X{Ed} 7 MCWButt()

  // --------------------------------------------------------------
  // Public globals.

  AliRunLoader* pRunLoader;

#include "ZAliLoad.h7"
  ClassDef(ZAliLoad, 1)
}; // endclass ZAliLoad

GlassIODef(ZAliLoad);

#endif
