// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_VSDSelector_H
#define Alice_VSDSelector_H

#include <Glasses/ZNode.h>
#include <Glasses/MCTrack.h>
#include <Glasses/HitContainer.h>
#include <Glasses/RecTrack.h>
#include <Glasses/TPCSegment.h>
#include <Glasses/GIImportStyle.h>

#include <Stones/TTreeTools.h>
#include <Stones/MCParticle.h>
#include <Stones/V0.h>
#include <Stones/GenInfo.h>
#include <Stones/Hit.h>
#include <Stones/Kink.h>

#include <Glasses/ZNode.h>

class VSDSelector : public ZNode
{
  MAC_RNR_FRIENDS(VSDSelector);
  friend class ZAliLoad;

 private:
  void _init();

  TTree*                         mTreeK;   // X{g}
  TTree*                         mTreeH;   // X{g}
  TTree*                         mTreeTR;  // X{g}
  TTree*                         mTreeC;   // X{g}
  TTree*                         mTreeR;   // X{g}
  TTree*                         mTreeV0;  // X{g}
  TTree*                         mTreeKK;  // X{g}
  TTree*                         mTreeGI;  // X{g}
  Hit                            mH, *mpH;     // needed for selection in mTreeH
  MCParticle                     mP, *mpP;     // needed for selection in mTreeK
  Hit                            mC, *mpC;     // needed for selection in mTreeC    
  ESDParticle                    mR, *mpR;     // needed for selection in mTreeR  
  V0                             mV0,*mpV0;    // needed for selection in mTreeV0  
  Kink                           mKK,*mpKK;    // needed for selection in mTreeV0  
  GenInfo                        mGI,*mpGI;    // needed for selection in mTreeGI      

 protected:
  TFile*                         mFile;      // X{gs} 
  TDirectory*                    mDirectory; // X{gs}
  
 public:
  VSDSelector(const Text_t* n="VSDSelector", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  void ResetEvent();  
  void LoadVSD(const Text_t* vsd_file_name);

  // --------------------------------------------------------------
  // Kinematics

 protected:
  TString    mParticleSelection;    // X{GS} 7 Textor(-whenchanged=>1)
 public:
  void SelectParticles( ZNode* holder=0, const Text_t* selection=0,
			Bool_t import_daughters=false
			);           // X{Ed} C{1} 7 MCWButt()

  MCParticle* Particle(Int_t i);
  // --------------------------------------------------------------
  // Hits 

 protected:
  TString mHitSelection;       // X{GS} 7 Textor(-whenchanged=>1)
 public:
  void SelectHits(ZNode* holder=0, const Text_t* selection=0
		  );           // X{Ed} C{1} 7 MCWButt()

  // --------------------------------------------------------------
  // Clusters

 protected:
  TString mClusterSelection;  // X{GS} 7 Textor(-whenchanged=>1)
 public:                            
  void SelectClusters(ZNode* holder=0, const Text_t* selection=0
		      );      // X{Ed} C{1} 7 MCWButt()

  // --------------------------------------------------------------
  // ESD

 protected:
  TString mRecSelection;      // X{GS} 7 Textor(-whenchanged=>1)
 public:
  void SelectRecTracks(ZNode* holder=0, const Text_t* selection=0
		       );     // X{Ed} C{1} 7 MCWButt()
  
  // --------------------------------------------------------------
  // V0 points
 protected:
  TString mV0Selection;        // X{GS} 7 Textor(-whenchanged=>1)
 public:
  void SelectV0( ZNode* holder=0, const Text_t* selection=0,
		 Bool_t import_kine=false);            // X{Ed} C{1} 7 MCWButt()
  // --------------------------------------------------------------
  // Kinks
 protected:
  TString mKinkSelection;        // X{GS} 7 Textor(-whenchanged=>1)
 public:
  void SelectKinks(ZNode* holder=0, const Text_t* selection=0,
		   Bool_t import_kine=false,
		   Bool_t import_daughters=false);     // X{Ed} C{1} 7 MCWButt()


  // --------------------------------------------------------------
  // GenInfo

 protected:
  ZLink<GIImportStyle> mImportMode;   // X{GS} L{}
  TString        mGISelection;  // X{GS} 7 Textor(-whenchanged=>1)
 public:
  void SelectGenInfo(ZNode* holder=0, const Text_t* selection=0
		     );         // X{Ed} C{1} 7 MCWButt()

#include "VSDSelector.h7"
  ClassDef(VSDSelector, 1)
    }; // endclass VSDSelector


#endif
