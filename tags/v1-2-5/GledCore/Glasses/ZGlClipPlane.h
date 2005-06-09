// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlClipPlane_H
#define GledCore_ZGlClipPlane_H

#include <Glasses/ZRnrModBase.h>

class ZGlClipPlane : public ZRnrModBase {
  // 7777 RnrCtrl(RnrBits(0,4,0,0))
  MAC_RNR_FRIENDS(ZGlClipPlane);

private:
  void _init();

protected:
  Float_t fDist;  // X{GS} 7 Value(-range=>[-1000,1000, 1,1000])
  Float_t fTheta; // X{GS} 7 Value(-range=>[-90,90,     1,1000], -join=>1)
  Float_t fPhi;   // X{GS} 7 Value(-range=>[-360,360,   1,1000])

  Bool_t  bRnrSelf; // X{GS} 7 Bool()
  Bool_t  bOnIfOff; // X{GS} 7 Bool(-join=>1)
  Bool_t  bOffIfOn; // X{GS} 7 Bool()

public:
  ZGlClipPlane(const Text_t* n="ZGlClipPlane", const Text_t* t=0) :
    ZRnrModBase(n,t) { _init(); }

#include "ZGlClipPlane.h7"
  ClassDef(ZGlClipPlane, 1)
}; // endclass ZGlClipPlane

GlassIODef(ZGlClipPlane);

#endif
