// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ShellInfo_H
#define Gled_ShellInfo_H

#include <Glasses/NestInfo.h>

class ShellInfo : public NestInfo {
  // 7777 RnrCtrl("true, 7, RnrBits(2,4,6,0, 0,0,0,5)")
  MAC_RNR_FRIENDS(ShellInfo);

private:
  void _init();

protected:
  ZList*	mNests;		// X{GS} L{l}
  ZGlass*	mBeta;		// X{GS} L{}
  ZGlass*	mGamma;		// X{GS} L{}

  Bool_t	bSpawnNests;	// X{GS} 7 Bool()
public:
  ShellInfo(Text_t* n="ShellInfo", Text_t* t=0) : NestInfo(n,t) { _init(); }

  virtual void AdEnlightenment();

#include "ShellInfo.h7"
  ClassDef(ShellInfo, 1)
}; // endclass ShellInfo

GlassIODef(ShellInfo);

#endif
