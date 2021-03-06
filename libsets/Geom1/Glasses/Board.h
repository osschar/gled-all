// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_Board_H
#define Geom1_Board_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>

class ZImage;

class Board : public ZNode {
  MAC_RNR_FRIENDS(Board);

protected:
  void _init();

  ZColor	mColor;		// X{PGST} 7 ColorButt()

  ZLink<ZImage>	mTexture;	// X{gST} L{} RnrBits{4,0,5,0, 0,0,0,0}
  Float_t	mTexX0;         // X{gST} 7 Value(-range=>[-1e3,1e3,1,1000], -join=>1)
  Float_t	mTexY0;         // X{gST} 7 Value(-range=>[-1e3,1e3,1,1000])
  Float_t	mTexX1;         // X{gST} 7 Value(-range=>[-1e3,1e3,1,1000], -join=>1)
  Float_t	mTexY1;         // X{gST} 7 Value(-range=>[-1e3,1e3,1,1000])

  Float_t	mULen;		// X{gST}  7 Value(-range=>[0,1e6,1,1000],-join=>1)
  Float_t	mVLen;		// X{gST}  7 Value(-range=>[0,1e6,1,1000])
  Int_t         mUDivs;         // X{GST}  7 Value(-range=>[0,1024,1], -join=>1)
  Int_t         mVDivs;         // X{GST}  7 Value(-range=>[0,1024,1])

public:
  Board(const Text_t* n="Board", const Text_t* t=0) :
    ZNode(n,t) { _init(); }
  virtual ~Board() {}

  void SetUV(Float_t u, Float_t v)
  { mULen = u; mVLen = v; Stamp(FID()); }  // X{E}

#include "Board.h7"
  ClassDef(Board, 1); // Textured rectangle in x-y plane.
}; // endclass Board


#endif
