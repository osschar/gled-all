// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZRlFont_H
#define GledCore_ZRlFont_H

#include <Glasses/ZRnrModBase.h>
#include <Net/Ray.h>

class ZRlFont : public ZRnrModBase {
  // 7777 RnrCtrl(RnrBits(0,4,0,0))
  MAC_RNR_FRIENDS(ZRlFont);

private:
  enum PrivRayQN_e  { PRQN_offset = RayNS::RQN_user_0,
		      PRQN_font_change
  };

  void _init();

protected:
  TString	mFontFile;    // X{GE}  7 Filor()
  Int_t         mSize;        // X{GST} 7 Value(-range=>[1,512,1])

public:
  ZRlFont(const Text_t* n="ZRlFont", const Text_t* t=0) :
    ZRnrModBase(n,t) { _init(); }

  void SetFontFile(const Text_t* f);

#include "ZRlFont.h7"
  ClassDef(ZRlFont, 1)
}; // endclass ZRlFont

GlassIODef(ZRlFont);

#endif
