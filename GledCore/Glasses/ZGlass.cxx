// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// ZGlass
//
// Base of Gled enabled classes.
// Provides infrastructure for integration with the Gled system.
//

#include "ZGlass.h"

#include <Ephra/Saturn.h>
#include <Glasses/ZQueen.h>
#include <Glasses/ZKing.h>
#include <Stones/ZMIR.h>
#include <Stones/ZComet.h>

ClassImp(ZGlass)

/**************************************************************************/

void ZGlass::_init()
{
  mSaturn=0; mQueen = 0; mMir=0;
  mSaturnID=0; bMIRActive = true; mRefCount=0;
  mTimeStamp=0;
  mStamp_CB=mStampLink_CB=0; mStamp_CBarg=mStampLink_CBarg=0;
}

/**************************************************************************/

void ZGlass::SetNameTitle(const Text_t* n, const Text_t* t)
{
  mName = n; mTitle = t;
  Stamp(LibID(), ClassID());
}

/**************************************************************************/

bool ZGlass::IsSunSpace()
{
  // Returns true if this lens is in local Saturn's sun-space.

  ZKing* sk = mSaturn->GetKing();
  return (mSaturnID >= sk->GetMinID() && mSaturnID <= sk->GetMaxID());
}

bool ZGlass::IsSunOrFireSpace()
{
  // Returns true if this lens is in local Saturn's sun or fire-space.

  return (mSaturnID >= mSaturn->GetKing()->GetMinID());
}

/**************************************************************************/

Short_t ZGlass::IncRefCount()
{
  mRefCountMutex.Lock();
  ++mRefCount;
  mRefCountMutex.Unlock();
  Stamp(LibID(), ClassID());
  return mRefCount;
}
Short_t ZGlass::DecRefCount()
{
  mRefCountMutex.Lock();
  --mRefCount;
  if(mRefCount==0 && mQueen) mQueen->ZeroRefCount(this);
  mRefCountMutex.Unlock();
  Stamp(LibID(), ClassID());
  return mRefCount;
}

/**************************************************************************/

// All stamp functions should be *inline*!!
// And reporting to a queen, not saturn.
// Queen can than filter out RQN_change requests for *very* dynamic
// parts of the scene in a case of constant-redraw rendering.

TimeStamp_t ZGlass::Stamp(LID_t lid, CID_t cid)
{
  ++mTimeStamp;
  if(mQueen) {
    Ray r(Ray::RQN_change, mTimeStamp, this, lid, cid);
    mQueen->EmitRay(r);
  }
  if(mStamp_CB) mStamp_CB(this, mStamp_CBarg);

  return mTimeStamp;
}

TimeStamp_t ZGlass::StampLink(LID_t lid, CID_t cid)
{
  ++mTimeStamp;
  if(mQueen) {
    Ray r(Ray::RQN_link_change, mTimeStamp, this, lid, cid);
    mQueen->EmitRay(r);
  }
  if(mStampLink_CB) mStampLink_CB(this, mStampLink_CBarg);

  return mTimeStamp;
}

void ZGlass::SetStamp_CB(zglass_stamp_f foo, void* arg)
{
  mStamp_CB = foo; mStamp_CBarg = arg;
}
void ZGlass::SetStampLink_CB(zglass_stamp_f foo, void* arg)
{
  mStampLink_CB = foo; mStampLink_CBarg = arg;
}

/**************************************************************************/
/*
TBuffer& operator<<(TBuffer& b, ZGlass* n) {
  if(n) {
    b << n->GetSaturnID();
    ISdebug(D_STREAM, _s<<"Writing id of "<< n->GetName() <<", ID="<< n->GetSaturnID()));
  } else {
    b << 0u;
    ISdebug(D_STREAM, _s<<"Writing id 0"));
  }
  return b;
}
*/

/**************************************************************************/

#include "ZGlass.c7"
