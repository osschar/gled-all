// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZMirEmittingEntity_H
#define GledCore_ZMirEmittingEntity_H

#include <Glasses/ZGlass.h>
#include <Glasses/ZIdentity.h>
#include <Glasses/ZHashList.h>

class SaturnInfo;

class ZMirEmittingEntity : public ZGlass {
  // 7777 RnrCtrl(0)
  MAC_RNR_FRIENDS(ZMirEmittingEntity);

  friend class Gled; friend class Saturn; friend class ZSunQueen;

private:
  void _init();

protected:
  TString		mLogin;			// X{GS} 7 TextOut()
  ZIdentity*		mPrimaryIdentity;	// X{GS} L{}
  ZHashList*		mActiveIdentities;	// X{GS} L{}

public:
  ZMirEmittingEntity(const Text_t* n="ZMirEmittingEntity", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }

  virtual void AdEnlightenment();

  virtual SaturnInfo* HostingSaturn() = 0;
  virtual void Message(const Text_t* s) {} // X{E}
  virtual void Error(const Text_t* s)   {} // X{E}

  Bool_t HasIdentity(ZIdentity* ident);

#include "ZMirEmittingEntity.h7"
  ClassDef(ZMirEmittingEntity, 1)
}; // endclass ZMirEmittingEntity

GlassIODef(ZMirEmittingEntity);

#endif
