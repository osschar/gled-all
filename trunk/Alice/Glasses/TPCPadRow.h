// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_TPCPadRow_H
#define Alice_TPCPadRow_H

#include <Glasses/TPCSegment.h>
#include <Gled/GledNS.h>
#include <Stones/ZColor.h>
#include <TH1I.h>

class TPCPadRow : public ZNode {
  MAC_RNR_FRIENDS(TPCPadRow);

 private:
  void _init();
  TPCSegment*         mSegment;    // X{gS} L{} 

  Float_t             mDx;         // X{GST} 7 Value(-range=>[0,100,1])
  Float_t             mDy;         // X{GST} 7 Value(-range=>[0,100,1])

  AliSimDigits*       mDigits;     // X{G}  

  Int_t               mRow;        // X{gE} 7 Value(-range=>[0,159,1])
  Int_t               mPad;        // X{GST}

  //text
  ZColor              mFgCol; // X{GSPT} 7 ColorButt(-join=>1)
  ZColor              mBgCol; // X{GSPT} 7 ColorButt()
  Int_t               mTX;     // X{GST}  7 Value(-range=>[-1000,1000,1], -join=>1)
  Int_t               mTY;     // X{GST}  7 Value(-range=>[-1000,1000,1], -join=>1)
  Float_t             mTZ;     // X{GST}  7 Value(-range=>[0,1,1,1000])


  TH1I*               mHistogram;  // X{GS}

 public:
  TPCPadRow(const Text_t* n="TPCPadRow", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  void SetRow(Int_t row);
  void PrintPad();

#include "TPCPadRow.h7"
  ClassDef(TPCPadRow, 1)
    }; // endclass TPCPadRow

GlassIODef(TPCPadRow);

#endif
