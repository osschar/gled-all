// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGroupIdentity_H
#define GledCore_ZGroupIdentity_H

#include <Glasses/ZIdentity.h>
#include <Glasses/ZNameMap.h>

class ZGroupIdentity : public ZIdentity {
  // 7777 RnrCtrl("false, 0, RnrBits()")
  MAC_RNR_FRIENDS(ZGroupIdentity);

  friend class ZSunQueen;

private:
  void _init();

protected:
  ZNameMap*	mActiveMMEs;	// X{GS} L{}

public:
  ZGroupIdentity(const Text_t* n="ZGroupIdentity", const Text_t* t=0) :
    ZIdentity(n,t) { _init(); }

  virtual void AdEnlightenment();

#include "ZGroupIdentity.h7"
  ClassDef(ZGroupIdentity, 1)
}; // endclass ZGroupIdentity

GlassIODef(ZGroupIdentity);

#endif
