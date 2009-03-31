// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
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
  Float_t mX;     // X{GS} 7 Value(-range=>[-1e3,1e3, 1,1000], -width=>6, -join=>1)
  Float_t mY;     // X{GS} 7 Value(-range=>[-1e3,1e3, 1,1000], -width=>6, -join=>1)
  Float_t mZ;     // X{GS} 7 Value(-range=>[-1e3,1e3, 1,1000], -width=>6)
  Float_t mDist;  // X{GS} 7 Value(-range=>[-1e3,1e3, 1,1000], -width=>6, -join=>1)
  Float_t mTheta; // X{GS} 7 Value(-range=>[-90,90,   1,1000], -width=>6, -join=>1)
  Float_t mPhi;   // X{GS} 7 Value(-range=>[-360,360, 1,1000], -width=>6)

  Bool_t  bRnrSelf; // X{GS} 7 Bool()
  Bool_t  bOnIfOff; // X{GS} 7 Bool(-join=>1)
  Bool_t  bOffIfOn; // X{GS} 7 Bool()

public:
  ZGlClipPlane(const Text_t* n="ZGlClipPlane", const Text_t* t=0) :
    ZRnrModBase(n,t) { _init(); }

#include "ZGlClipPlane.h7"
  ClassDef(ZGlClipPlane, 1);
}; // endclass ZGlClipPlane


#endif
