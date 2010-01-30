// $Header$
// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_ZAliLoad_H
#define Alice_ZAliLoad_H

#include <Glasses/ZNode.h>
#include <Glasses/VSDSelector.h>

class ZAliLoad : public ZNode
{
  MAC_RNR_FRIENDS(ZAliLoad);

 private:
  void _init();
  bool m_auto_vsdfile_p; //!

 protected:
  Bool_t  check_read(const TString& file);
  TString get_vsd_name(Bool_t check_p);

  ZLink<VSDSelector>             mSelector;   // X{GS} L{}

  TString                        mDataDir;    // X{GS} 7 Filor(-dir=>1, -whenchanged=>1)
  Int_t                          mEvent;      // X{GS} 7 Value(-range=>[0,100,1])

  TString                        mDefVSDName; // X{GS} 7 Textor(-whenchanged=>1)
  TString                        mVSDFile;    // X{GS} 7 Filor(-whenchanged=>1)
  
  TString                        mOperation; //! X{GS} 7 TextOut()
  GMutex                         mOpMutex;   //! X{r}
 public:
  ZAliLoad(const Text_t* n="ZAliLoad", const Text_t* t=0) :
    ZNode(n,t), mOpMutex(GMutex::recursive) { _init(); }

  void LoadVSD();     // X{Ed} 7 MButt()
  void ClearData();   // X{Ed} 7 MButt()

  // -------------------------------------------------------------- 
  // Wrapper functions for selections. 
 protected:
  TString    mParticleSelection;    // X{GS} 7 Textor(-whenchanged=>1)
 public:
  void SelectParticles(const Text_t* selection=0,
		       Bool_t import_daughters=false); // X{Ed} 7 MCWButt()

 protected:
  TString mHitSelection;       // X{GS} 7 Textor(-whenchanged=>1)
 public:
  void SelectHits(const Text_t* selection=0);          // X{Ed} 7 MCWButt()

 protected:
  TString mClusterSelection;  // X{GS} 7 Textor(-whenchanged=>1)
 public:  
  void SelectClusters(const Text_t* selection=0);      // X{Ed} 7 MCWButt()

 protected:
  TString mRecSelection;      // X{GS} 7 Textor(-whenchanged=>1)
 public:
  void SelectRecTracks(const Text_t* selection=0);     // X{Ed} 7 MCWButt()

 protected:
  TString mV0Selection;       // X{GS} 7 Textor(-whenchanged=>1)
 public:
  void SelectV0(const Text_t* selection=0,
		Bool_t import_kine=false);             // X{Ed} 7 MCWButt()

 protected:
  TString mKinkSelection;       // X{GS} 7 Textor(-whenchanged=>1)
 public:
  void SelectKinks(const Text_t* selection=0,
		   Bool_t import_kine=false,
		   Bool_t import_daughters=false);     // X{Ed} 7 MCWButt()

 protected:
  TString        mGISelection;  // X{GS} 7 Textor(-whenchanged=>1)
 public:
  void SelectGenInfo(const Text_t* selection=0);       // X{Ed} 7 MCWButt()

#include "ZAliLoad.h7"
  ClassDef(ZAliLoad, 1);
}; // endclass ZAliLoad

#endif
