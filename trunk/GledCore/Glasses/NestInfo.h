// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_NestInfo_H
#define Gled_NestInfo_H

#include <Glasses/ZList.h>

class NestInfo : public ZList {
  // 7777 RnrCtrl("false, 0, RnrBits(2,4,6,0, 0,0,0,5)")
  MAC_RNR_FRIENDS(NestInfo);

private:
  void _init();

protected:
  ZList*	mPupils;	// X{GS} L{l}

  Bool_t	bSpawnPupils;	// X{GS} 7 Bool()

public:
  NestInfo(const Text_t* n="NestInfo", const Text_t* t=0) :
    ZList(n,t) { _init(); }

  virtual void AdEnlightenment();

  void ImportKings(); // X{E}

#include "NestInfo.h7"
  ClassDef(NestInfo, 1)
}; // endclass NestInfo

GlassIODef(NestInfo);

#endif
