// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZIdentity_H
#define GledCore_ZIdentity_H

#include <Glasses/ZGlass.h>
#include <Glasses/ZHashList.h>

class ZIdentity : public ZGlass {
  MAC_RNR_FRIENDS(ZIdentity);

  friend class ZSunQueen;

private:
  void _init();

protected:
  ZHashList*		mActiveMMEs;	// X{gS} L{}

  ZMirFilter*		mAllowThis;	// X{gS} L{}

public:
  ZIdentity(const Text_t* n="ZIdentity", const Text_t* t=0) : ZGlass(n,t) { _init(); }

  virtual void AdEnlightenment();

#include "ZIdentity.h7"
  ClassDef(ZIdentity, 1) // Representation of a user identity
}; // endclass ZIdentity

GlassIODef(ZIdentity);

#endif
