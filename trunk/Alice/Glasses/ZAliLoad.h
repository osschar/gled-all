// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_ZAliLoad_H
#define Alice_ZAliLoad_H

#include <Glasses/ZNode.h>
#include <Glasses/VSDSelector.h>
#include <Glasses/AliConverter.h>

#include <Stones/TPCDigitsInfo.h>
#include <Stones/ITSDigitsInfo.h>

class ZAliLoad : public ZNode {
  MAC_RNR_FRIENDS(ZAliLoad);

 private:
  void _init();
  bool m_auto_vsdfile_p; //!

 protected:
  Bool_t check_read(const string& file);
  string get_vsd_name(Bool_t check_p);

  AliConverter*                  mConverter;  // X{GS} L{}
  VSDSelector*                   mSelector;   // X{GS} L{}

  TString                        mDataDir;    // X{GS} 7 Filor(-dir=>1, -whenchanged=>1)
  Int_t                          mEvent;      // X{GS} 7 Value(-range=>[0,100,1])
  AliConverter::KineType_e       mKineType;   // X{GS} 7 PhonyEnum() 

  TString                        mDefVSDName; // X{GS} 7 Textor(-whenchanged=>1)
  TString                        mVSDFile;    // X{GS} 7 Filor(-whenchanged=>1)
  
  TString                        mOperation; //! X{GS} 7 TextOut()
  GMutex                         mOpMutex;   //! X{r}

 public:
  ZAliLoad(const Text_t* n="ZAliLoad", const Text_t* t=0) :
    ZNode(n,t), mOpMutex(GMutex::recursive) { _init(); }

  void CreateVSD();   // X{Ed} 7 MButt()
  void LoadVSD();     // X{Ed} 7 MButt()
  void ClearData();   // X{Ed} 7 MButt()

  // -------------------------------------------------------------- 
  // Wrapper functions for selections. 
 protected:
  TString mParticleSelection;  // X{GS} 7 Textor(-whenchanged=>1)
 public:
  void SelectParticles(const Text_t* selection=0,
		       Bool_t import_daughters=false); // X{Ed} 7 MCWButt()

 protected:
  TString mHitSelection;       // X{GS} 7 Textor(-whenchanged=>1)
 public:
  void SelectHits(const Text_t* selection=0);          // X{Ed} 7 MCWButt()

 protected:
  TString mClusterSelection;   // X{GS} 7 Textor(-whenchanged=>1)
 public:  
  void SelectClusters(const Text_t* selection=0);      // X{Ed} 7 MCWButt()

 protected:
  TString mRecSelection;       // X{GS} 7 Textor(-whenchanged=>1)
 public:
  void SelectRecTracks(const Text_t* selection=0);     // X{Ed} 7 MCWButt()

 protected:
  TString mV0Selection;        // X{GS} 7 Textor(-whenchanged=>1)
 public:
  void SelectV0(const Text_t* selection=0,
		Bool_t import_kine=false);             // X{Ed} 7 MCWButt()
 protected:
  TString mGISelection;        // X{GS} 7 Textor(-whenchanged=>1)
 public:
  void SelectGenInfo(const Text_t* selection=0);       // X{Ed} 7 MCWButt()

  // --------------------------------------------------------------
  // Digits. 
 protected:
  void check_tpcdig_info();
  TPCDigitsInfo* mTPCDigInfo; // X{g}
 public:
  TPCSegment* ShowTPCSegment(Int_t segment_id, ZNode* holder = 0);  
  void        ShowTPCPlate(Int_t side = -1);   // X{Ed} 7 MCWButt()

 protected:
  void check_itsdig_info();
  ITSDigitsInfo* mITSDigInfo; // X{g}
 public:
  void        ShowITSModule(Int_t module, ZNode* holder=0);
  void        ShowITSDet(Int_t subdet = -1, Bool_t show_empty = false);// X{Ed} 7 MCWButt()

#include "ZAliLoad.h7"
  ClassDef(ZAliLoad, 1)
}; // endclass ZAliLoad

GlassIODef(ZAliLoad);

#endif
