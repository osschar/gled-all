// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_NestInfo_H
#define GledCore_NestInfo_H

#include <Glasses/ZList.h>

class NestInfo : public ZList {
  // 7777 RnrCtrl(1)
  MAC_RNR_FRIENDS(NestInfo);

public:
  enum LeafLayout_e {
    LL_Ants,
    LL_Custom
  };

private:
  void _init();

protected:
  ZList*	mPupils;	// X{gS} L{l}

  Bool_t	bSpawnPupils;	// X{GS} 7 Bool()

  LeafLayout_e  mLeafLayout;    // X{GS} 7 PhonyEnum()
  TString       mLayout;        // X{GS} 7 Textor()

public:
  NestInfo(const Text_t* n="NestInfo", const Text_t* t=0) :
    ZList(n,t) { _init(); }

  virtual void AdEnlightenment();

  void EnactLayout(); // X{E} 7 MButt()
  void ImportKings(); // X{E} 7 MButt()

#include "NestInfo.h7"
  ClassDef(NestInfo, 1)
}; // endclass NestInfo

GlassIODef(NestInfo);

#endif
