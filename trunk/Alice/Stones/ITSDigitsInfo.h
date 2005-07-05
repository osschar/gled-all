// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_ITSDigitsInfo_H
#define Alice_ITSDigitsInfo_H

#include <map>

#include <Stones/ZRCStone.h>
#include <TClonesArray.h>

#include <AliITS.h>
#include <AliITSgeom.h>
#include <AliITSsegmentationSPD.h>
#include <AliITSsegmentationSDD.h>
#include <AliITSsegmentationSSD.h>

class ITSDigitsInfo : public ZRCStone
{
 private:
  void _init();
  Float_t mSPDZCoord[192];

 protected:
  TString                    mDataDir;  // X{G} 
  Int_t                      mEvent;    // X{G}

  map<Int_t,  TClonesArray*> mSPDmap;   //!
  map<Int_t,  TClonesArray*> mSDDmap;   //!
  map<Int_t,  TClonesArray*> mSSDmap;   //!
  void        SetITSSegmentation();

 public:
  TTree*                   mTree;
  AliITSgeom*              mGeom; 
  AliITSsegmentationSPD*   mSegSPD; 
  AliITSsegmentationSDD*   mSegSDD; 
  AliITSsegmentationSSD*   mSegSSD; 

  ITSDigitsInfo(const Text_t* n="ITSDigitsInfo", const Text_t* t=0) :
    ZRCStone()
  {_init(); } 
 
  virtual ~ITSDigitsInfo();

  void SetData(const Text_t* data_dir, Int_t event);
 
  TClonesArray* GetDigits(Int_t moduleID, Int_t detector);

  void    GetSPDLocalZ(Int_t j, Float_t& z);
  void    Dump();

#include "ITSDigitsInfo.h7"
  ClassDef(ITSDigitsInfo, 1)
    }; // endclass ITSDigitsInfo

#endif
