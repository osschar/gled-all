// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZRlFont_H
#define GledCore_ZRlFont_H

#include <Glasses/ZGlass.h>

class ZRlFont : public ZGlass {
  MAC_RNR_FRIENDS(ZRlFont);

private:
  void _init();

protected:
  TString	mFontName;    // X{GS}  7 Textor()

public:
  ZRlFont(const Text_t* n="ZRlFont", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }


#include "ZRlFont.h7"
  ClassDef(ZRlFont, 1)
}; // endclass ZRlFont

GlassIODef(ZRlFont);

#endif
