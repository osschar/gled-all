// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_RecTrackRS_H
#define Alice_RecTrackRS_H

#include <Glasses/PRSBase.h>
#include <Stones/ZColor.h>

class RecTrackRS : public PRSBase
{
  MAC_RNR_FRIENDS(RecTrackRS);

 private:
  void _init();

 public:
  Bool_t      bFitKinks;      // X{GST}  7 Bool( -join=>1)
  Bool_t      bFitV0;         // X{GST}  7 Bool()
  Float_t     mKinkVSize;     // X{GST}  7 Value(-range=>[0.1,64, 1,10])
  ZColor      mKinkVColor;    // X{PGST} 7 ColorButt()

 public:
  RecTrackRS(const Text_t* n="RecTrackRS", const Text_t* t=0) :
    PRSBase(n,t) { _init(); }

#include "RecTrackRS.h7"
  ClassDef(RecTrackRS, 1)
}; // endclass RecTrackRS


#endif
