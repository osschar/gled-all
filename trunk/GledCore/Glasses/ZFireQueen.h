// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZFireQueen_H
#define GledCore_ZFireQueen_H

#include <Glasses/ZQueen.h>

class ZFireQueen : public ZQueen {
  // 7777 RnrCtrl("false, 0, RnrBits()")
  MAC_RNR_FRIENDS(ZFireQueen);

private:
  void _init();

protected:
  ZHashList*	mEunuchs;	// X{GS} L{}

public:
  ZFireQueen(const Text_t* n="ZFireQueen", const Text_t* t=0) :
    ZQueen(n,t) { _init(); }

  virtual void AdEnlightenment();

#include "ZFireQueen.h7"
  ClassDef(ZFireQueen, 1)
}; // endclass ZFireQueen

GlassIODef(ZFireQueen);

#endif
