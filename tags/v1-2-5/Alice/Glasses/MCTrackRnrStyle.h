// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_MCTrackRnrStyle_H
#define Alice_MCTrackRnrStyle_H

#include <Glasses/PRSBase.h>
#include <Stones/ZColor.h>
#include <Glasses/MCTrack.h>
#include <Glasses/ZImage.h>

class MCTrackRnrStyle : public PRSBase {
  // 7777 RnrCtrl(RnrBits(0,4,0,0))
  MAC_RNR_FRIENDS(MCTrackRnrStyle);
  MAC_RNR_FRIENDS(MCTrack);

 private:
  void _init();

  // protected:
 public:
  Bool_t                   mFitDaughters;  // X{GST}  7 Bool(-join=>1)
  Bool_t                   mFitDecay;      // X{GST}  7 Bool()])

  // PDG colors
  ZColor   mDefCol;       // X{GSPT} 7 ColorButt(-join=>1)
  ZColor   mECol;         // X{GSPT} 7 ColorButt()
  ZColor   mMuCol;        // X{GSPT} 7 ColorButt(-join=>1)
  ZColor   mGammaCol;     // X{GSPT} 7 ColorButt()
  ZColor   mMesCol;       // X{GSPT} 7 ColorButt(-join=>1)
  ZColor   mBarCol;       // X{GSPT} 7 ColorButt()


  // public:
  MCTrackRnrStyle(const Text_t* n="MCTrackRnrStyle", const Text_t* t=0) :
    PRSBase(n,t) { _init(); }
  ZColor                   GetPdgColor(Int_t pdg);
  ZImage*                  GetPdgTexture(Int_t pdg);

#include "MCTrackRnrStyle.h7"
  ClassDef(MCTrackRnrStyle, 1)
    }; // endclass MCTrackRnrStyle

GlassIODef(MCTrackRnrStyle);

#endif
