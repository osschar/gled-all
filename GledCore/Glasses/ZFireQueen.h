// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZFireQueen_H
#define GledCore_ZFireQueen_H

#include <Glasses/ZQueen.h>
class ZEunuch;

class ZFireQueen : public ZQueen
{
  MAC_RNR_FRIENDS(ZFireQueen);

private:
  void _init();

protected:
  ZLink<ZHashList>	mEunuchs;	// X{gS} L{}

  virtual void bootstrap();

public:
  ZFireQueen(const Text_t* n="ZFireQueen", const Text_t* t=0) :
    ZQueen(n, t) { _init(); }
  ZFireQueen(ID_t span, const Text_t* n="ZFireQueen", const Text_t* t=0) :
    ZQueen(span, n, t) { _init(); }

  // ID & Lens management
  virtual ZGlass* DemangleID(ID_t id);

#include "ZFireQueen.h7"
  ClassDef(ZFireQueen, 1)
}; // endclass ZFireQueen


#endif
