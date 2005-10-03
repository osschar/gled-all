// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ScreenText_H
#define GledCore_ScreenText_H

#include <Glasses/ZGlass.h>
#include <Stones/ZColor.h>

class ScreenText : public ZGlass {
  MAC_RNR_FRIENDS(ScreenText);

private:
  void _init();

protected:
  TString	mText;  // X{GS}  7 Textor()
  ZColor	mFgCol; // X{GSP} 7 ColorButt(-join=>1)
  ZColor	mBgCol; // X{GSP} 7 ColorButt()
  Int_t		mX;     // X{GS}  7 Value(-range=>[-1000,1000,1], -join=>1)
  Int_t		mY;     // X{GS}  7 Value(-range=>[0,100,1], -join=>1)
  Float_t	mZ;     // X{GS}  7 Value(-range=>[0,1,1,1000])

public:
  ScreenText(const Text_t* n="ScreenText", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }


#include "ScreenText.h7" // Text display in window coordinates.
  ClassDef(ScreenText, 1)
}; // endclass ScreenText


#endif
