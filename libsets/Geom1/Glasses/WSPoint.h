// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
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
  friend class WSTube;

private:
  void _init();

protected:
  Float_t	mW; // Width   X{GS} 7 Value(-range=>[-1e3,1e3,1,1000], -width=>6, -join=>1)
  Float_t	mS; // Spread  X{GS} 7 Value(-range=>[-1e3,1e3,1,1000], -width=>6, -join=>1)
  Float_t	mT; // Tension X{GS} 7 Value(-range=>[-1e3,1e3,1,1000], -width=>6)

  Float_t	mTwist;   // Texture wraps X{GS} 7 Value(-range=>[-1e3,1e3,1,1000], -join=>1)
  Float_t	mStretch; // Lenght        X{GE} 7 Value(-range=>[-1e3,1e3,1,1000])

  // Internal stuff for connections.
  TMatrixD mCoffs;     //! X{R}
  WSPoint* mCoffPoint; //!
  WSPoint* mPrevPoint; //!
  WSPoint* mNextPoint; //!

public:
  WSPoint(const Text_t* n="WSPoint", const Text_t* t=0) :
    ZNode(n,t), mCoffs(4,4) { _init(); }

  void Coff(WSPoint* f);

  // Stamping, custom set-methods.

  virtual TimeStamp_t Stamp(FID_t fid, UChar_t eye_bits=0);

  void SetStretch(Float_t stretch);

  void InsertPoint(Float_t time=0.5); // X{E} 7 MCWButt()

#include "WSPoint.h7"
  ClassDef(WSPoint, 1);
}; // endclass WSPoint


#endif
