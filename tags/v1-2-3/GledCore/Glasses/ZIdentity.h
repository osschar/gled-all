// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZIdentity_H
#define GledCore_ZIdentity_H

#include <Glasses/ZGlass.h>
#include <Glasses/ZHashList.h>

class ZIdentity : public ZGlass {
  // 7777 RnrCtrl("false, 0, RnrBits()")
  MAC_RNR_FRIENDS(ZIdentity);

  friend class ZSunQueen;

private:
  void _init();

protected:
  ZHashList*		mActiveMMEs;	// X{GS} L{}

  ZMirFilter*		mAllowThis;	// X{GS} L{}

public:
  ZIdentity(const Text_t* n="ZIdentity", const Text_t* t=0) : ZGlass(n,t) { _init(); }

  virtual void AdEnlightenment();

#include "ZIdentity.h7"
  ClassDef(ZIdentity, 1) // Representation of a user identity
}; // endclass ZIdentity

GlassIODef(ZIdentity);

#endif
