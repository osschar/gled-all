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
// mGlassBits: collection of flags that allow (optimised) handling of lenses.

#include "ZGlass.h"

#include <Ephra/Saturn.h>
#include <Glasses/ZQueen.h>
#include <Glasses/ZKing.h>
#include <Glasses/ZMirFilter.h>
#include <Stones/ZMIR.h>
#include <Stones/ZComet.h>

/**************************************************************************/

namespace ZGlassBits {
  UShort_t kFixedName = 0x1;
}

/**************************************************************************/

ClassImp(ZGlass)

/**************************************************************************/

void ZGlass::_init()
{
  mSaturn = 0; mQueen = 0; mMir = 0;
  mGlassBits = 0; mSaturnID=0;
  mGuard = 0;
  bMIRActive = true;
  mRefCount = mMoonRefCount = mSunRefCount = mFireRefCount = 0; 
  mTimeStamp = mStampReqTring = 0;
  pSetYNameCBs = 0;
  mStamp_CB=mStampLink_CB=0; mStamp_CBarg=mStampLink_CBarg=0;
}

/**************************************************************************/

typedef set<YNameChangeCB*>		spYNameChangeCB_t;	
typedef set<YNameChangeCB*>::iterator	spYNameChangeCB_i;	

void ZGlass::register_name_change_cb(YNameChangeCB* rec)
{
  if(mGlassBits & ZGlassBits::kFixedName) return;
  mRefCountMutex.Lock();
  if(pSetYNameCBs == 0)
    pSetYNameCBs = new set<YNameChangeCB*>;
  pSetYNameCBs->insert(rec);
  mRefCountMutex.Unlock();
}

void ZGlass::unregister_name_change_cb(YNameChangeCB* rec)
{
  mRefCountMutex.Lock();
  if(pSetYNameCBs != 0)
    pSetYNameCBs->erase(rec);
  mRefCountMutex.Unlock();
}


void ZGlass::SetName(const Text_t* n)
{
  if(mGlassBits & ZGlassBits::kFixedName) {
    Stamp(LibID(), ClassID());
    throw(string("ZGlass::SetName lens has FixedName bit set"));
  }
  string name(n);
  {
    string::size_type i;
    while((i = name.find_first_of('/')) != string::npos)
      name.replace(i, 1, 1, '_');
  }
  mExecMutex.Lock();
  mRefCountMutex.Lock();
  if(pSetYNameCBs != 0) {
    for(set<YNameChangeCB*>::iterator i=pSetYNameCBs->begin(); i!=pSetYNameCBs->end(); ++i) {
      (*i)->y_name_change_cb(this, name);
    }
  }
  mRefCountMutex.Unlock();
  mName = name.c_str();
  Stamp(LibID(), ClassID());
  mExecMutex.Unlock();
}

void ZGlass::SetNameTitle(const Text_t* n, const Text_t* t)
{
  SetName(n);
  SetTitle(t);
}

ZMirFilter* ZGlass::GetGuard() const
{
  mExecMutex.Lock();
  ZMirFilter* _ret = mGuard;
  mExecMutex.Unlock();
  return _ret;
}

void ZGlass::SetGuard(ZMirFilter* guard)
{
  mExecMutex.Lock();
  if(mGuard) mGuard->DecRefCount(this);
  mGuard = guard;
  StampLink(LibID(), ClassID());
  if(mGuard) mGuard->IncRefCount(this);
  mExecMutex.Unlock();
}

/**************************************************************************/

ZGlass::~ZGlass()
{
  // !!!!! Should unref all links; check zlist, too
  delete pSetYNameCBs;
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

Short_t ZGlass::IncRefCount(const ZGlass* from)
{
  if(from->mQueen) {
    mRefCountMutex.Lock();
    ++mRefCount;
    switch(from->mQueen->GetKing()->GetLightType()) {
    case ZKing::LT_Moon:++mMoonRefCount; break;
    case ZKing::LT_Sun:	++mSunRefCount;  break;
    case ZKing::LT_Fire:++mFireRefCount; break;
    default: ISerr("ZGlass::IncRefCount King in undefined state");
    }
    mRefCountMutex.Unlock();
    Stamp(LibID(), ClassID());
  }
  return mRefCount;
}

Short_t ZGlass::DecRefCount(const ZGlass* from)
{
  if(from->mQueen) {
    mRefCountMutex.Lock();
    --mRefCount;
    switch(from->mQueen->GetKing()->GetLightType()) {
    case ZKing::LT_Moon:--mMoonRefCount; break;
    case ZKing::LT_Sun:	--mSunRefCount;  break;
    case ZKing::LT_Fire:--mFireRefCount; break;
    default: ISerr("ZGlass::IncRefCount King in undefined state");
    }
    if(mRefCount==0 && mQueen) mQueen->ZeroRefCount(this);
    mRefCountMutex.Unlock();
    Stamp(LibID(), ClassID());
  }
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

void ZGlass::AssertMIRPresence(const string& header, int what)
{
  if(mMir == 0) {
    throw(header + "should be called with mMir set");
  }
  if((what & MC_IsBeam) && !mMir->HasRecipient()) {
    throw(header + "should be called via a beamed mir");
  }
  if((what & MC_HasResultReq) && !mMir->HasResultReq()) {
    throw(header + "should be called with result request set");
  }
}

/**************************************************************************/

#include "ZGlass.c7"
