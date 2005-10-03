// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_MetaWeedInfo_H
#define GledCore_MetaWeedInfo_H

#include <Glasses/ZGlass.h>
#include <Stones/ZColor.h>

class MetaWeedInfo : public ZGlass {
  MAC_RNR_FRIENDS(MetaWeedInfo);

public:
  enum BoxType_e { BT_None=0, 
		   BT_Flat, BT_Up, BT_ThinUp, BT_Engraved, BT_Border
  };

private:
  void _init();

protected:
  Int_t		mX;      // X{GS} 7 Value(-range=>[0,256,1], -join=>1)
  Int_t		mY;      // X{GS} 7 Value(-range=>[0,256,1])
  Int_t		mW;      // X{GS} 7 Value(-range=>[0,256,1], -join=>1)
  Int_t		mH;      // X{GS} 7 Value(-range=>[0,256,1])

  Bool_t	bColorP; // X{GS}  7 Bool(-join=>1)
  ZColor	mColor;  // X{PGS} 7 ColorButt()

  Bool_t	bLabelP; // X{GS}  7 Bool()

  Bool_t	bAlignP;  // X{GS} 7 Bool(-join=>1)
  Bool_t	bAInside; // X{GS} 7 Bool()
  Char_t	mALtRt;   // X{GS} 7 Value(-range=>[-1,1,1], -join=>1)
  Char_t	mAUpDn;   // X{GS} 7 Value(-range=>[-1,1,1])

  BoxType_e	mBoxType; // X{GS} 7 PhonyEnum()
  Bool_t	bBDown;   // X{GS} 7 Bool(-join=>1)
  Bool_t	bBFrame;  // X{GS} 7 Bool()

public:
  MetaWeedInfo(const Text_t* n="MetaWeedInfo", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }

  void Resize(int x, int y, int w, int h);         // X{E}
  void Color(Float_t r, Float_t g, Float_t b);     // X{E}
  void Label(const Text_t* l);                     // X{E}
  void Align(Bool_t inside, Char_t lr, Char_t ud); // X{E}
  void Box(BoxType_e box, Bool_t downp=false, Bool_t framep=false); // X{E}

#include "MetaWeedInfo.h7"
  ClassDef(MetaWeedInfo, 1)
}; // endclass MetaWeedInfo


#endif
