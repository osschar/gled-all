// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_Board_H
#define Geom1_Board_H

#include <Glasses/ZNode.h>
#include <Glasses/ZImage.h>
#include <Stones/ZColor.h>

class Board : public ZNode {
  MAC_RNR_FRIENDS(Board);

protected:
  void _init();

  ZImage*	mTexture;	// X{gS} L{} RnrBits{4,0,5,0, 0,0,0,0}
  Float_t	mTexX0;         // X{gS} 7 Value(-range=>[-1e3,1e3,1,1000], -join=>1)
  Float_t	mTexY0;         // X{gS} 7 Value(-range=>[-1e3,1e3,1,1000])
  Float_t	mTexX1;         // X{gS} 7 Value(-range=>[-1e3,1e3,1,1000], -join=>1)
  Float_t	mTexY1;         // X{gS} 7 Value(-range=>[-1e3,1e3,1,1000])

  Float_t	mULen;		// X{gS}  7 Value(-range=>[0,1000,1,100],-join=>1)
  Float_t	mVLen;		// X{gS}  7 Value(-range=>[0,1000,1,100])
  bool		bFillBack;	// X{gS}  7 Bool(-join=>1);
  ZColor	mColor;		// X{PGS} 7 ColorButt()

public:
  Board(const Text_t* n="Board", const Text_t* t=0) :
    ZNode(n,t) { _init(); }
  virtual ~Board() {}

  void SetUV(Float_t u, Float_t v)
  { mULen = u; mVLen = v; Stamp(FID()); }  // X{E}

#include "Board.h7"
  ClassDef(Board, 1)
}; // endclass Board

GlassIODef(Board);

#endif
