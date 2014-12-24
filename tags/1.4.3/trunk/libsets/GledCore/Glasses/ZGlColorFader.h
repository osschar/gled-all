// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlColorFader_H
#define GledCore_ZGlColorFader_H

#include <Glasses/ZRnrModBase.h>
#include <Stones/ZColor.h>

class ZGlColorFader : public ZRnrModBase
{
  // 7777 RnrCtrl(RnrBits(0,4,0,0))
  MAC_RNR_FRIENDS(ZGlColorFader);

private:
  void _init();

protected:
  ZColor  mColorFac;     // X{PRGS} 7 ColorButt()
  Float_t mPointSizeFac; // X{GS}   7 Value(-range=>[0,128, 1,1000])
  Float_t mLineWidthFac; // X{GS}   7 Value(-range=>[0,128, 1,1000])

public:
  ZGlColorFader(const Text_t* n="ZGlColorFader", const Text_t* t=0) :
    ZRnrModBase(n,t) { _init(); }



#include "ZGlColorFader.h7"
  ClassDef(ZGlColorFader, 1);
}; // endclass ZGlColorFader


#endif
