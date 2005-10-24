// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_WSPoint_H
#define Geom1_WSPoint_H

#include <Glasses/ZNode.h>
#include <TMatrixD.h>

class WSPoint : public ZNode
{
  MAC_RNR_FRIENDS(WSPoint);
  MAC_RNR_FRIENDS(WSSeed);
  friend class WSSeed;

private:
  void _init();

  TMatrixD	mCoffs;         //! X{R}

protected:
  Float_t	mW;		// Width   X{GS} 7 Value(-range=>[-100,100,1,1000], -width=>6, -join=>1)
  Float_t	mS;		// Spread  X{GS} 7 Value(-range=>[-100,100,1,1000], -width=>6, -join=>1)
  Float_t	mT;		// Tension X{GS} 7 Value(-range=>[-100,100,1,1000], -width=>6)

  Float_t	mTwist;		// X{GS} 7 Value(-range=>[-100,100,1,1000], -join=>1)
  Float_t	mStretch;	// X{GS} 7 Value(-range=>[-100,100,1,1000])

  bool		bDrawAxen;	// X{GS} 7 Bool()

public:
  WSPoint(const Text_t* n="WSPoint", const Text_t* t=0) : 
    ZNode(n,t), mCoffs(4,4) { _init(); }

  void Coff(const WSPoint* f);

  // virtuals
  virtual TimeStamp_t Stamp(FID_t fid, UChar_t eye_bits=0);

#include "WSPoint.h7"
  ClassDef(WSPoint, 1)
}; // endclass WSPoint


#endif
