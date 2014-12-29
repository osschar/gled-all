// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
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

#include <Glasses/ZQueen.h>

#include <TMath.h>

ClassImp(WSPoint);

/**************************************************************************/

void WSPoint::_init()
{
  mW = 0.1; mS = 0; mT = 1;
  mTwist = 0; mStretch = 1;

  mCoffPoint = mPrevPoint = mNextPoint = 0;
}

/**************************************************************************/

namespace
{
  inline Double_t sqr(Double_t x) { return x*x; }
}

void WSPoint::Coff(WSPoint* f)
{
  if (f == mCoffPoint)
    return;

  const ZTrans& a = RefTrans();
  const ZTrans& b = f->RefTrans();

  const Double_t *AT = a.ArrT(), *AX = a.ArrX();
  const Double_t *BT = b.ArrT(), *BX = b.ArrX();

  const Double_t d =
    TMath::Sqrt(sqr(AT[0] - BT[0]) + sqr(AT[1] - BT[1]) + sqr(AT[2] - BT[2]));
  const Double_t T1 = mT*d, T2 = f->mT*d;

  for (Int_t i=0; i<3; ++i)
  {
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
  {
    const Double_t A = f->mS - 2*(f->mW - mW) + mS;
    TMatrixDRow row( mCoffs[3] );
    row[0] = mW;
    row[1] = mS;
    row[2] = (f->mW - mW) - mS - A;
    row[3] = A;
  }

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

/**************************************************************************/

void WSPoint::InsertPoint(Float_t time)
{
  // Directly instantiates a new WSPoint on time-position 'time'.
  // Presumably this point is in parent-list exactly once (point is
  // added where first found + next point is correct).
  //
  // As point creation is potentially cluster-wide MIR presence is reqired
  // as well as common queen with parent.


  static const Exc_t _eh("WSPoint::InsertPoint ");

  suggest_MIR_presence(_eh, ZGlass::MC_IsFlare);

  if(*mParent == 0)
    throw(_eh + "parent not set.");
  if(mQueen != mParent->GetQueen())
    throw(_eh + "queen is not shared with parent.");

  if(mNextPoint == 0)
    throw(_eh + "can not insert at last point.");

  Int_t insert_id = -1;
  { bool found = false;
    GMutexHolder llck(mParent->RefListMutex());
    for(ZList::iterator i=mParent->begin(); i!=mParent->end(); ++i) {
      if(i->fLens == this) {
        ++i;
        if(i != mParent->end() && i->fLens == mNextPoint) {
          insert_id = i->fId;
          found = true;
          break;
        }
      }
    }
    if(!found)
      throw(_eh + "can not find insertion point.");
  }

  Coff(mNextPoint);

  WSPoint* p = new WSPoint("New point");
  const Double_t t = time, t2 = t*t, t3 = t2*t;
  {
    ZTrans& lcf = p->ref_trans();

    Double_t* Pnt = lcf.ArrT();
    Double_t* Axe = lcf.ArrX();

    for(Int_t i=0; i<3; i++) {
      const TMatrixDRow R( mCoffs[i] );
      Pnt[i] = R[0] +   R[1]*t +   R[2]*t2 + R[3]*t3;
      Axe[i] = R[1] + 2*R[2]*t + 3*R[3]*t2;
    }

    lcf.OrtoNorm3();
  }
  {
    const TMatrixDRow W( mCoffs[3] );
    p->mW = W[0] + W[1]*t + W[2]*t2 + W[3]*t3;
  }
  {
    GLensWriteHolder wlck(*mParent);
    mQueen->CheckIn(p);
    mParent->InsertById(p, insert_id);
  }
}
