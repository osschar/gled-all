// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlPerspective_H
#define GledCore_ZGlPerspective_H

#include <Glasses/ZRnrModBase.h>

class ZGlPerspective : public ZRnrModBase
{
  // 7777 RnrCtrl(RnrBits(0,4,0,0))
  MAC_RNR_FRIENDS(ZGlPerspective);

public:
  enum ViewMode_e { VM_Nop, VM_OrthoPixel, VM_OrthoFixed, VM_OrthoTrueAspect };

private:
  void _init();

protected:
  ViewMode_e        mViewMode;     // X{GS} 7 PhonyEnum()
  Float_t           mOrthoW;       // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mOrthoH;       // X{GS} 7 Value(-range=>[-1000,1000,1,1000])
  Float_t           mOrthoNear;    // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mOrthoFar;     // X{GS} 7 Value(-range=>[-1000,1000,1,1000])

  Float_t           mOx;           // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mOy;           // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mOz;           // X{GS} 7 Value(-range=>[-1000,1000,1,1000])

public:
  ZGlPerspective(const Text_t* n="ZGlPerspective", const Text_t* t=0) :
    ZRnrModBase(n,t) { _init(); }

  void StandardPersp();      //  X{E} 7 MButt(-join=>1)
  void StandardFixed();      //  X{E} 7 MButt()
  void StandardTrueAspect(); //  X{E} 7 MButt(-join=>1)
  void StandardPixel();      //  X{E} 7 MButt()

#include "ZGlPerspective.h7"
  ClassDef(ZGlPerspective, 1);
}; // endclass ZGlPerspective


#endif
