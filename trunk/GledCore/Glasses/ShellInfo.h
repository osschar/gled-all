// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ShellInfo_H
#define Gled_ShellInfo_H

#include <Glasses/NestInfo.h>

class ShellInfo : public NestInfo {
  MAC_RNR_FRIENDS(ShellInfo);

private:
  void _init();

protected:
  ZList*	mNests;		// X{gS} L{l}
  ZGlass*	mBeta;		// X{gS} L{}
  ZGlass*	mGamma;		// X{gS} L{}

  Bool_t	bSpawnNests;	// X{gS} 7 Bool()
public:
  ShellInfo(const Text_t* n="ShellInfo", const Text_t* t=0) :
    NestInfo(n,t) { _init(); }

  virtual void AdEnlightenment();

#include "ShellInfo.h7"
  ClassDef(ShellInfo, 1)
}; // endclass ShellInfo

GlassIODef(ShellInfo);

#endif
