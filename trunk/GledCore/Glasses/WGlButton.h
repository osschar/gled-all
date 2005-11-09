// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_WGlButton_H
#define GledCore_WGlButton_H

#include <Glasses/ZNode.h>
#include <Gled/GledNS.h>

class WGlButton : public ZNode
{
  MAC_RNR_FRIENDS(WGlButton);

private:
  void _init();

protected:
  Float_t   mDx; // X{GST} 7 Value(-range=>[0,1000,1,1000], -join=>1)
  Float_t   mDy; // X{GST} 7 Value(-range=>[0,1000,1,1000])

  ZLink<ZGlass>       mCbackAlpha;      //  X{GS} L{} Ray{CbackReset}
  TString             mCbackMethodName; //  X{GS}     Ray{CbackReset} 7 Textor()
  GledNS::MethodInfo* mCbackMethodInfo; //!

public:
  WGlButton(const Text_t* n="WGlButton", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  GledNS::MethodInfo* GetCbackMethodInfo();
  void EmitCbackResetRay() { mCbackMethodInfo = 0; }

  void MenuEnter(); // X{E}
  void MenuExit();  // X{E}

#include "WGlButton.h7"
  ClassDef(WGlButton, 1)
}; // endclass WGlButton


#endif
