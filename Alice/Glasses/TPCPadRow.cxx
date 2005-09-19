// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TPCPadRow
//
//

#include "TPCPadRow.h"
#include "TPCPadRow.c7"

#include <GledView/GledGUI.h>
#include <TPad.h>

ClassImp(TPCPadRow)

/****************************w**********************************************/

  void TPCPadRow::_init()
{
  // *** Set all links to 0 ***
  bUseDispList = true;
  mRow = 0;
  mPad = 0;
  mSegment = 0;

  mDx = 70;
  mDy = 4;

  mDigits = 0; 
  mBgCol.rgba(0,0,0,0.5);
  mTX = 0; mTY = 45;  mTZ = 1e-3;

  mHistogram = 0;
}


void TPCPadRow::SetRow(Int_t row)
{
  mRow = row;
  mPad = 0;
  mStampReqTring = Stamp(FID());
}

void TPCPadRow::PrintPad()
{
  if(mDigits == 0) return; 
  Bool_t done,found;
  done = found = false;
  Int_t time, pad, val;

  mDigits->First();  
  if(mPad ==  mDigits->CurrentColumn()) found = true;

  mHistogram = new TH1I(GForm("Signal in time"), GForm("Pad %d ",mPad), 446, -0.5, 445.5);
  do {
    time = mDigits->CurrentRow();
    pad  = mDigits->CurrentColumn();
    val  = mDigits->CurrentDigit();
    if(mDigits->Next()) {
      if(mPad == mDigits->CurrentColumn()){
	// printf("time %d, pad %d , val %d \n",
	//       mDigits->CurrentRow(), mDigits->CurrentColumn(),  mDigits->CurrentDigit());
	mHistogram->Fill(mDigits->CurrentRow(), mDigits->CurrentDigit());
	found = true;
      } else if (found == true) 
	break;
    } else {
      done = true;
    }
  } while (!done);
  if(gPad == 0)
    GledGUI::theOne->NewCanvas();
  mHistogram->Draw();
  gPad->Update();
}
