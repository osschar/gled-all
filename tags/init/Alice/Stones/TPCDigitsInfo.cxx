// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TPCDigitsInfo
//
//

#include "TPCDigitsInfo.h"

using namespace std;

ClassImp(TPCDigitsInfo)


/**************************************************************************/

void TPCDigitsInfo::_init()
{
  mTree = 0;
  mParameter= 0;
}

TPCDigitsInfo::~TPCDigitsInfo()
{
}
/**************************************************************************/
void TPCDigitsInfo::SetData(AliTPCParam* par, TTree* tree)
{ 
  mParameter = par;
  mSegEnt.assign(72,-1);
  mTree = tree;

 
  AliSimDigits *digit=&mSimDigits;
  mTree->GetBranch("Segment")->SetAddress(&digit);
  
  Int_t sbr=(Int_t)mTree->GetEntries();
  for (Int_t ent=0; ent<sbr; ent++) {
    mTree->GetEntry(ent);
    Int_t s, row;
    par->AdjustSectorRow(digit->GetID(),s,row);
    // printf("found entry %d in sec %d row %d \n",ent, s, row);

    if(row == 0) mSegEnt[s] = ent;
  }


  // read TPC Seg data
  mInnSeg.pad_width = par->GetInnerPadPitchWidth();
  mInnSeg.pad_length = par->GetInnerPadPitchLength();
  mInnSeg.Rlow = par->GetPadRowRadiiLow(0);
  mInnSeg.nRows = par->GetNRowLow();
  mInnSeg.nMaxPads = par->GetNPadsLow(mInnSeg.nRows -1);


  mOut1Seg.pad_width = par->GetOuterPadPitchWidth();
  mOut1Seg.pad_length = par->GetOuter1PadPitchLength();
  mOut1Seg.Rlow = par->GetPadRowRadiiUp(0);
  mOut1Seg.nRows = par->GetNRowUp1();
  mOut1Seg.nMaxPads =  par->GetNPadsUp(mOut1Seg.nRows-1);
 

  mOut2Seg.pad_width =  par->GetOuterPadPitchWidth();
  mOut2Seg.pad_length =par->GetOuter2PadPitchLength();
  mOut2Seg.Rlow = par->GetPadRowRadiiUp(mOut1Seg.nRows);
  mOut2Seg.nRows = par->GetNRowUp() - mOut1Seg.nRows;
  mOut2Seg.nMaxPads = par->GetNPadsUp(par->GetNRowUp()-1);


  // set stepsize array
  Int_t k, npads;
  // Inn
  k=0, npads = par->GetNPadsLow(0);
  for (int row = 0; row < mInnSeg.nRows ;row++ ){
    if (par->GetNPadsLow(row) > npads){
      npads = par->GetNPadsLow(row);
      mInnSeg.stepy[k]=row* mInnSeg.pad_length +mInnSeg.Rlow;
      k++;
    }
  }
  mInnSeg.nsteps=k;
  // Out1 seg 
  k=0; npads = par->GetNPadsUp(0);
  for (int row = 0; row < mOut1Seg.nRows ;row++ ){
    if (par->GetNPadsUp(row) > npads){
      npads = par->GetNPadsUp(row);
      mOut1Seg.stepy[k]=row* mOut1Seg.pad_length + mOut1Seg.Rlow ;
      k++;
    }
  }
  mOut1Seg.nsteps=k;
  // Out2 seg
  k=0; npads = par->GetNPadsUp(mOut1Seg.nRows);
  for (int row = mOut1Seg.nRows; row < par->GetNRowUp() ;row++ ){
    if (par->GetNPadsUp(row) > npads){
      npads = par->GetNPadsUp(row);
      mOut2Seg.stepy[k]=(row - mOut1Seg.nRows)* mOut2Seg.pad_length + mOut2Seg.Rlow ;
      k++;
    }
  }
  mOut2Seg.nsteps=k;
}
