// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_TPCDigitsInfo_H
#define Alice_TPCDigitsInfo_H

#include <vector>

#include <TObject.h>
#include <TArrayI.h>
#include <TTree.h>

#include <AliTPCParam.h>
#include <AliSimDigits.h>

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


class TPCDigitsInfo : public TObject {
 private:
  void _init();

 protected:

 public:
  AliSimDigits        mSimDigits;
  AliTPCParam*        mParameter;
  TTree*              mTree;
  std::vector<Int_t>  mSegEnt;
  TPCSeg              mInnSeg;
  TPCSeg              mOut1Seg;
  TPCSeg              mOut2Seg;

  TPCDigitsInfo(const Text_t* n="TPCDigitsInfo", const Text_t* t=0) :
    TObject() { _init(); }
  virtual ~TPCDigitsInfo();

  void SetData(AliTPCParam* par, TTree* tree);

#include "TPCDigitsInfo.h7"
  ClassDef(TPCDigitsInfo, 1)
    }; // endclass TPCDigitsInfo

#endif
