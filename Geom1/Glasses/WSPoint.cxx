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

ClassImp(WSPoint);

/**************************************************************************/

void WSPoint::_init()
{
  mW = 0.1; mS = 0; mT = 1;
  mTwist = 0; mStretch = 1;

  mCoffPoint = mPrevPoint = mNextPoint = 0;
}

/**************************************************************************/

namespace {
  inline Double_t sqr(Double_t x) { return x*x; }
}

void WSPoint::Coff(WSPoint* f)
{
  if(f == mCoffPoint)
    return;

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

    TMatrixDRow row( mCoffs[i] );
    row[0] = AT[i];
    row[1] = Q;
    row[2] = P - Q - R;
    row[3] = R;
  }
  // Widths
  const Double_t A = f->mS - 2*(f->mW - mW) + mS;
  TMatrixDRow row( mCoffs[3] );
  row[0] = mW;
  row[1] = mS;
  row[2] = (f->mW - mW) - mS - A;
  row[3] = A;

  mCoffPoint = f;
}

/**************************************************************************/

TimeStamp_t WSPoint::Stamp(FID_t fid, UChar_t eye_bits)
{
  // Upon change also change parent's triangulation stamp (if it is a WSSeed).
  // Usually WSSeed is stored as a single display-list.

  TimeStamp_t t = PARENT_GLASS::Stamp(fid, eye_bits);
  if(fid.is_null() || fid == ZNode::FID() || fid == FID())
  {
    mCoffPoint = 0;
    if(mPrevPoint)
      mPrevPoint->mCoffPoint = 0;

    WSSeed* seed = dynamic_cast<WSSeed*>(*mParent);
    if(seed)
      seed->MarkStampReqTring();
  }
  return t;
}

/**************************************************************************/

void WSPoint::SetStretch(Float_t stretch)
{
  if(stretch > 100) stretch = 100;
  if(stretch < -100) stretch = -100;
  if(mNextPoint) {
    WSSeed* seed = dynamic_cast<WSSeed*>(*mParent);
    if(seed)
      seed->SetTrueLength(seed->GetTrueLength() + stretch - mStretch);
  }
  mStretch = stretch;
  Stamp(FID());
}
