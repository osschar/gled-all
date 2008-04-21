// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_WGlWidget_H
#define GledCore_WGlWidget_H

#include <Glasses/ZNode.h>

class WGlWidget : public ZNode
{
  MAC_RNR_FRIENDS(WGlWidget);

private:
  void _init();

protected:
  Float_t   mDx; // X{GST} 7 Value(-range=>[0,1000,1,1000], -join=>1)
  Float_t   mDy; // X{GST} 7 Value(-range=>[0,1000,1,1000])

  ZLink<ZGlass>   mCbackAlpha;      //  X{GS} L{} Ray{CbackReset}

public:
  WGlWidget(const Text_t* n="WGlWidget", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  virtual void EmitCbackResetRay() {}

  virtual void SetCbackBeta(ZGlass* lens) {}

  void SetDaughterCbackAlpha(ZGlass* lens, Int_t recurse_lvl=0); // X{E} C{1} 7 MCWButt()
  void SetDaughterCbackStuff(ZGlass* lens, Int_t recurse_lvl=0); // X{E} C{1} 7 MCWButt()

#include "WGlWidget.h7"
  ClassDef(WGlWidget, 1);
}; // endclass WGlWidget


#endif
