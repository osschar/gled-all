// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZNodeLink_H
#define Gled_ZNodeLink_H

#include <Glasses/ZNode.h>

class ZNodeLink : public ZNode {
  // 7777 RnrCtrl("true, true, RnrBits(2,0,6,0, 0,0,0,5)")
  MAC_RNR_FRIENDS(ZNodeLink);
private:
  void		_init() 	{ mGlass = 0; }
protected:
  ZGlass*	mGlass;		// X{gS} L{} RnrBits{0,0,0,4}
public:
  ZNodeLink(Text_t* n="ZNodeLink", Text_t* t=0) : ZNode(n,t) {_init();}
#include "ZNodeLink.h7"

  ClassDef(ZNodeLink, 1)
}; // endclass ZNodeLink

GlassIODef(ZNodeLink);

#endif
