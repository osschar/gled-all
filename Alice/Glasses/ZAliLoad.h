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
#include <Glasses/RecTrack.h>
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

 public:
  enum KineType_e { KT_Standard, KT_ProtonProton };

 private:
  void                           _init();
  ZParticle*                     create_particle_with_parents(Int_t label, Int_t depth = -1);
  TTree*                         mTreeK;   // X{g}
  TTree*                         mTreeH;   // X{g}
  TTree*                         mTreeTR;  // X{g}
  TTree*                         mTreeC;   // X{g}
  TTree*                         mTreeR;   // X{g}
  Hit                            mH, *mpH;      // needed for selection in mTreeH
  MCParticle                     mP, *mpP;      // needed for selection in mTreeK
  Hit                            mC, *mpC;      // needed for selection in mTreeC    
  ESDTrack                       mR, *mpR;      // needed for selection in mTreeR                   
  TPCDigitsInfo*                 mTPCDigInfo;

 protected:
  ZParticle* get_track(Int_t tid);
  AliTPCParam* get_tpc_param(const string& eh);

  TString                  mDataDir;   // X{GS} 7 Filor(-dir=>1)
  Int_t			   mEvent;     // X{GS} 7 Value(-range=>[0,100,1])
  KineType_e               mKineType;  // X{GS} 7 PhonyEnum()

  TString	           mVSDName;   // X{GS} 7 Textor()

  TFile*                   mFile;      // X{G} 
  TDirectory*              mDirectory; // X{G}

  TString                  mOperation; //! X{GS} 7 TextOut()
  GMutex                   mOpMutex;   //! X{r}

 public:
  ZAliLoad(const Text_t* n="ZAliLoad", const Text_t* t=0);

  void SetupDataSource(Bool_t use_aliroot=false); // X{Ed} 7 MCWButt()

  void SetupEvent(Bool_t use_aliroot);

  void WriteVSD();      // X{Ed} 7 MButt()

  void ResetEvent(); // X{Ed} 7 MButt()

  // --------------------------------------------------------------
  // Kinematics

 protected:
  TString    mParticleSelection; // X{GS} 7 Textor()
 public:
  void LoadKinematics();
  void SelectParticles(ZNode* holder=0, const Text_t* selection=0,
		       Bool_t import_daughters=false
		       );        // X{Ed} C{1} 7 MCWButt()

  MCParticle* Particle(Int_t i);
  void        PrintTreeK();

  // --------------------------------------------------------------
  // Hits 

 protected:
  TString    mHitSelection; // X{GS} 7 Textor()
 public:
  void LoadHits();
  void SelectHits(HitContainer* holder=0, const Text_t* selection=0
		  );        // X{Ed} C{1} 7 MCWButt()

  // --------------------------------------------------------------
  // Clusters

 protected:
  TString     mClusterSelection; // X{GS} 7 Textor()
 public:
  void        LoadClusters();
  void        SelectClusters(HitContainer* holder=0, const Text_t* selection=0
			     );  // X{Ed} C{1} 7 MCWButt()

  // --------------------------------------------------------------
  // ESD

 protected:
  TString     mRecTrackSelection;        // X{GS} 7 Textor()
 public:
  void        LoadRecTracks();
  void        SelectRecTracks(ZNode* holder=0, const Text_t* selection=0
			      );         // X{Ed} C{1} 7 MCWButt()
  void        SelectRecTracksWithKine(ZNode* holder=0,
				      const Text_t* selection=0, Int_t depth = 0
				      ); // X{Ed} C{1} 7 MCWButt()
  ZNode*      mRecTrackList;

  // --------------------------------------------------------------
  // GenInfo
  void         MakeGenInfo();

  // --------------------------------------------------------------
  // TPC specific  (digits,clusters)
  TPCSegment* ShowTPCSegment(Int_t segment_id, ZNode* holder = 0);  
  void        ShowTPCPlate(Int_t side = -1);   // X{Ed} 7 MCWButt()
  void        LoadTPCClusters();


  // --------------------------------------------------------------
  // Public globals.

  AliRunLoader* pRunLoader;

#include "ZAliLoad.h7"
  ClassDef(ZAliLoad, 1)
}; // endclass ZAliLoad

GlassIODef(ZAliLoad);

#endif
