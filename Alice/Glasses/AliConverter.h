// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_AliConverter_H
#define Alice_AliConverter_H

#include <Glasses/ZGlass.h>
#include <Glasses/ZQueen.h>
#include <Glasses/RecTrack.h>
#include <Glasses/V0Track.h>

#include <Stones/TTreeTools.h>
#include <Stones/GenInfo.h>
#include <Stones/Hit.h>
#include <Stones/MCParticle.h>
#include <Stones/Kink.h>
#include <Stones/V0.h>

#include <TBranchElement.h>
#include <TEventList.h>
#include <TParticle.h>

#include <AliRun.h>
#include <AliTPCParam.h>

class AliConverter : public ZGlass
{
  MAC_RNR_FRIENDS(AliConverter);
 
 public:
  enum KineType_e { KT_Standard, KT_ProtonProton };

 private:
  void _init();

 protected:
  void                           make_its_digits_info();
  GenInfo*                       get_geninfo(Int_t label);
  AliTPCParam*                   get_tpc_param(const Exc_t& eh);

  KineType_e                     mKineType;  // X{GS} 7 PhonyEnum()
  TString                        mDataDir;   // X{G}
  Int_t                          mEvent;     // X{G}

  TTree*                         mTreeK;       //! X{g}
  TTree*                         mTreeH;       //! X{g}
  TTree*                         mTreeTR;      //! X{g}
  TTree*                         mTreeC;       //! X{g}
  TTree*                         mTreeV0;      //! X{g}
  TTree*                         mTreeKK;       //! X{g}
  TTree*                         mTreeR;       //! X{g}
  TTree*                         mTreeGI;      //! X{g}

  Hit                            mH,  *mpH;    //! needed for selection in mTreeH
  MCParticle                     mP,  *mpP;    //! needed for selection in mTreeK
  Hit                            mC,  *mpC;    //! needed for selection in mTreeC    
  ESDParticle                    mR,  *mpR;    //! needed for selection in mTreeR  
  V0                             mV0, *mpV0;   //! needed for selection in mTreeV0  
  Kink                           mKK, *mpKK;   //! needed for selection in mTreeV0  
  GenInfo                        mGI, *mpGI;   //! needed for selection in mTreeGI      

  TFile*                         mFile;      // X{gs} 
  TDirectory*                    mDirectory; // X{gs}

  Float_t                        mTRDHitRes;  // X{gs} 
  Float_t                        mTPCHitRes;  // X{gs} 

 public:
  AliConverter(const Text_t* n="AliConverter", const Text_t* t=0) :
    ZGlass(n,t) { _init();}
  // virtual ~AliConverter(){ delete mVSDFile; }

  void CreateVSD(const Text_t* data_dir, Int_t event,
		 const Text_t* vsd_file);  // X{Ed} 

  // --------------------------------------------------------------
  // Conversion functions.

  void ConvertKinematics();
  void ConvertHits();
  void ConvertClusters();
  void ConvertTPCClusters();
  void ConvertITSClusters();
  void ConvertV0();
  void ConvertKinks();
  void ConvertRecTracks();
  void ConvertGenInfo();

  // --------------------------------------------------------------
  // Globals.

  AliRunLoader* pRunLoader;

#include "AliConverter.h7"
  ClassDef(AliConverter, 1)
}; // endclass AliConverter


#endif
