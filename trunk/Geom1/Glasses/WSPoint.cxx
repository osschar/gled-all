// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// WSPoint
//
// Weaver Symbol Point. Represents one control point of a Weaver
// Symbol.  Its effect is determined by its position and orientation
// (from ZNode) and by three real parameters: mW (width), mS (spread)
// and mT (tension).

#include "WSPoint.h"
#include "WSSeed.h"
#include "WSPoint.c7"

ClassImp(WSPoint)

/**************************************************************************/

void WSPoint::_init()
{
  mW = 0.1; mS = 0; mT = 1;
  mTwist = 0; mStretch = 1;
  bDrawAxen = false;
}

namespace {
  inline Float_t sqr(Float_t x) { return x*x; }
}

void WSPoint::Coff(const WSPoint* f)
{
  const ZTrans& a = RefTrans();
  const ZTrans& b = f->RefTrans();
  Float_t d = sqrt(sqr(a(1,4) - b(1,4)) +
		   sqr(a(2,4) - b(2,4)) +
		   sqr(a(3,4) - b(3,4)));
  Float_t T1 = mT*d, T2 = f->mT*d;
  for(Int_t i=1; i<=3; i++) {
    Float_t A = T2*b(i, 1) - 2*(b(i, 4) - a(i, 4)) + T1*a(i, 1);
    mCoffs(i,0) = a(i, 4);
    mCoffs(i,1) = T1*a(i, 1);
    mCoffs(i,2) = (b(i, 4) - a(i, 4)) - T1*a(i, 1) - A;
    mCoffs(i,3) = A;
  }
  // Widths
  {
    Float_t A = f->mS - 2*(f->mW - mW) + mS;
    mCoffs(4,0) = mW;
    mCoffs(4,1) = mS;
    mCoffs(4,2) = (f->mW - mW) - mS - A;
    mCoffs(4,3) = A;
  }
}

/**************************************************************************/

TimeStamp_t WSPoint::Stamp(FID_t fid)
{
  // Upon change also change parent's triangulation stamp (if it is a WSSeed).
  // Not optimal as it forces re-computation of Coffs for all WSPoints
  // in the symbol.

  TimeStamp_t t = PARENT_GLASS::Stamp(fid);
  if(fid.is_null() || fid == ZNode::FID() || fid == FID())
  {
    mStampReqTring = t;

    WSSeed* seed = dynamic_cast<WSSeed*>(mParent);
    if(seed) seed->MarkStampReqTring();
  }

  return t;
}
