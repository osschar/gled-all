// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_Board_H
#define Geom1_Board_H

#include <Glasses/ZNode.h>
#include <Glasses/ZImage.h>
#include <Stones/ZColor.h>

// Thinks GL_RGB, GL_UNSIGNED_BYTE ... should be a parameter
// Now working on ZImage.

class Board : public ZNode {
  // 7777 RnrCtrl("true, 4, RnrBits(1,2,4,0, 0,0,0,3)")
  MAC_RNR_FRIENDS(Board);

private:
  void _init();

  ZImage*	mTexture;	// X{GS} L{} RnrBits{1,0,2,0, 0,0,0,0}

  Real_t	mULen;		// X{GS}  7 Value(-range=>[0,1000,1,100],-join=>1)
  Real_t	mVLen;		// X{GS}  7 Value(-range=>[0,1000,1,100])
  bool		bFillBack;	// X{GS}  7 Bool(-join=>1);
  ZColor	mColor;		// X{PGS} 7 ColorButt()

  UChar_t*	pData;		//!

public:
  Board(const Text_t* n="Board", const Text_t* t=0) :
    ZNode(n,t) { _init(); }
  virtual ~Board();

  void SetUV(Real_t u, Real_t v)
  { mULen = u; mVLen = v; Stamp(LibID(), ClassID()); }
  Int_t ReadFile();		// X{E} 77 MButt()

#include "Board.h7"
  ClassDef(Board, 1)
}; // endclass Board

GlassIODef(Board);

#endif
