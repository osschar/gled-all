// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_LampLink_H
#define Gled_LampLink_H

#include <Glasses/ZGlass.h>
#include <Glasses/Lamp.h>

class LampLink : public ZGlass {
  // 7777 RnrCtrl("false, 1, RnrBits(0,1,0,0)")
  MAC_RNR_FRIENDS(LampLink);

private:
  void _init();

protected:
  Lamp*		mLamp;	  // X{GS} L{} RnrBits zero
  Bool_t	bTurnOn;  // X{GS} 7 Bool(-join=>1)
  Bool_t	bTurnOff; // X{GS} 7 Bool()

public:
  LampLink(const Text_t* n="LampLink", const Text_t* t=0) : ZGlass(n,t) { _init(); }


#include "LampLink.h7"
  ClassDef(LampLink, 1)
}; // endclass LampLink

GlassIODef(LampLink);

#endif
