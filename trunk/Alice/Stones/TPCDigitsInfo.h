// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_TPCDigitsInfo_H
#define Alice_TPCDigitsInfo_H

#include <Stones/ZRCStone.h>
#include <TArrayI.h>
#include <TTree.h>

#include <AliTPCParam.h>
#include <AliSimDigits.h>

#include <vector>

struct TPCSeg {
  Float_t   pad_width, pad_length,Rlow; // vertices data
  Int_t     nRows;                      // text & vertices data 
  Int_t     nMaxPads;                   // texture offset data
  Float_t   stepy[64];                  // y coord wher npads has changed
  Int_t     nsteps;                     // number of steps

  void dump(){
    printf("TPCSeg>> pw %f, pl %f, R %f, nRows %d, nMaxPads %d \n",
	   pad_width, pad_length,Rlow,nRows,nMaxPads);
  }
};


class TPCDigitsInfo : public ZRCStone
{
 private:
  void _init();

 protected:
  TString             mDataDir;  // X{G} 
  Int_t               mEvent;    // X{G}

 public:
  AliSimDigits        mSimDigits;
  AliTPCParam*        mParameter;
  TTree*              mTree;
  std::vector<Int_t>  mSegEnt;
  TPCSeg              mInnSeg;
  TPCSeg              mOut1Seg;
  TPCSeg              mOut2Seg;

  TPCDigitsInfo(const Text_t* n="TPCDigitsInfo", const Text_t* t=0) :
    ZRCStone() { _init(); }
  virtual ~TPCDigitsInfo();

  void SetData(const Text_t* data_dir, Int_t event);

#include "TPCDigitsInfo.h7"
  ClassDef(TPCDigitsInfo, 1)
    }; // endclass TPCDigitsInfo

#endif
