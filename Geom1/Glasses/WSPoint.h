// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_WSPoint_H
#define Geom1_WSPoint_H

#include <Glasses/ZNode.h>

class WSPoint : public ZNode {

  // 7777 RnrCtrl("true, 7, RnrBits(2,4,6,0, 0,0,0,5)")
  MAC_RNR_FRIENDS(WSPoint);
  MAC_RNR_FRIENDS(WSSeed);
  friend class WSSeed;

private:
  void _init();
  ZMatrix	mCoffs;

protected:
  Real_t	mW;		// Width   X{GS} 7 Value(-range=>[-100,100,1,100], -width=>6, -join=>1)
  Real_t	mS;		// Spread  X{GS} 7 Value(-range=>[-100,100,1,100], -width=>6, -join=>1)
  Real_t	mT;		// Tension X{GS} 7 Value(-range=>[-100,100,1,100], -width=>6)

  Real_t	mTwist;		// X{GS} 7 Value(-range=>[-100,100,1,100], -join=>1)
  Real_t	mStretch;	// X{GS} 7 Value(-range=>[-100,100,1,100])

  bool		bDrawAxen;	// 	   X{GS} 7 Bool()

public:
  WSPoint(Text_t* n="WSPoint", Text_t* t=0) : ZNode(n,t), mCoffs(5,4) { _init(); }

  void Coff(const WSPoint* f);

  // virtuals
  virtual TimeStamp_t Stamp(LID_t lid=0, CID_t cid=0);

#include "WSPoint.h7"
  ClassDef(WSPoint, 1)
}; // endclass WSPoint

GlassIODef(WSPoint);

#endif
