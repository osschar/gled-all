// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
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

ClassImp(WSPoint)

/**************************************************************************/

void WSPoint::_init()
{
  mW = 0.1; mS = 0; mT = 1; bDrawAxen = false;
}

namespace {
  inline Real_t sqr(Real_t x) { return x*x; }
}

void WSPoint::Coff(const WSPoint* f)
{
  const ZTrans& a = RefTrans();
  const ZTrans& b = f->RefTrans();
  Real_t d = sqrt(sqr(a(1u,4u)-b(1u,4u)) +
		  sqr(a(2u,4u)-b(2u,4u)) +
		  sqr(a(3u,4u)-b(3u,4u)));
  Real_t T1 = mT*d, T2 = f->mT*d;
  for(UCIndex_t i=1; i<=3; i++) {
    Real_t A = T2*b(i, 1u) - 2*(b(i, 4u) - a(i, 4u)) + T1*a(i, 1u);
    mCoffs(i,0u) = a(i,4u);
    mCoffs(i,1u) = T1*a(i,1u);
    mCoffs(i,2u) = (b(i, 4u) - a(i, 4u)) - T1*a(i, 1u) - A;
    mCoffs(i,3u) = A;
  }
  // Widths
  {
    Real_t A = f->mS - 2*(f->mW - mW) + mS;
    mCoffs(4u,0u) = mW;
    mCoffs(4u,1u) = mS;
    mCoffs(4u,2u) = (f->mW - mW) - mS - A;
    mCoffs(4u,3u) = A;
  }
}

/**************************************************************************/

TimeStamp_t WSPoint::Stamp(LID_t lid, CID_t cid)
{
  // Upon change also change parent's triangulation stamp (if it is a WSSeed).
  // Not optimal as it forces re-computation of Coffs for all WSPoints
  // in the symbol.

  TimeStamp_t t = ZNode::Stamp(lid, cid);
  if(lid==0 && cid == 0 ||
     lid==ZNode::LibID() && cid==ZNode::ClassID() ||
     lid==LibID() && cid==ClassID())
  {
    mStampReqTring = t;

    WSSeed* seed = dynamic_cast<WSSeed*>(mParent);
    if(seed) seed->MarkStampReqTring();
  }

  return t;
}

#include "WSPoint.c7"
