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
  inline Double_t sqr(Double_t x) { return x*x; }
}

void WSPoint::Coff(const WSPoint* f)
{
  const ZTrans& a = RefTrans();
  const ZTrans& b = f->RefTrans();

  const Double_t *AT = a.ArrT(), *AX = a.ArrX();
  const Double_t *BT = b.ArrT(), *BX = b.ArrX();

  const Double_t d =
    TMath::Sqrt(sqr(AT[0] - BT[0]) + sqr(AT[1] - BT[1]) + sqr(AT[2] - BT[2]));
  const Double_t T1 = mT*d, T2 = f->mT*d;

  for(Int_t i=0; i<3; ++i) {
    const Double_t P = BT[i] - AT[i];
    const Double_t Q = T1*AX[i];
    const Double_t R = T2*BX[i] - 2*P + Q;

    mCoffs(i,0) = AT[i];
    mCoffs(i,1) = Q;
    mCoffs(i,2) = P - Q - R;
    mCoffs(i,3) = R;
  }
  // Widths
  const Double_t A = f->mS - 2*(f->mW - mW) + mS;
  mCoffs(3,0) = mW;
  mCoffs(3,1) = mS;
  mCoffs(3,2) = (f->mW - mW) - mS - A;
  mCoffs(3,3) = A;
}

/**************************************************************************/

TimeStamp_t WSPoint::Stamp(FID_t fid, UChar_t eye_bits)
{
  // Upon change also change parent's triangulation stamp (if it is a WSSeed).
  // Not optimal as it forces re-computation of Coffs for all WSPoints
  // in the symbol.

  TimeStamp_t t = PARENT_GLASS::Stamp(fid, eye_bits);
  if(fid.is_null() || fid == ZNode::FID() || fid == FID())
  {
    mStampReqTring = t;

    WSSeed* seed = dynamic_cast<WSSeed*>(mParent.get());
    if(seed) seed->MarkStampReqTring();
  }

  return t;
}
