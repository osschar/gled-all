// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.


//______________________________________________________________________
// ZList
//
//

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Need member-lid, member-cid limiters for Add methods
// This should also be used for Links to Lists ...
// there can specify member-lid, member-cid of list, that can be assigned.
// have: ZList*   mOperators; L{list<Operator>}, p7 translates that to 1, 8 (eg)
// SetOperators(ZList* x) { 
//   if(lid or cid differr) {
//	instantiate x->lid, x->cid
//	try dyn-casting to self.lid, self.cid
//   }
// But then can get more restrictive list-contents than originally
// anticipated.
//
// This in contrast to dis-activation of Glasses and providing
// Add/Remove wrappers (as in SaturnInfo or SunQueen)


#include "ZList.h"
#include <Ephra/Saturn.h>
#include <Net/Ray.h>
#include <Stones/ZComet.h>
#include <TBuffer.h>

#include <algorithm>
#include <iterator>

ClassImp(ZList)

/**************************************************************************/

void ZList::_init()
{
  mSize = 0;
  mStampListAdd_CB = 0;		mStampListAdd_CBarg = 0;
  mStampListRemove_CB = 0;	mStampListRemove_CBarg = 0;
}

/**************************************************************************/

void ZList::UnrefAll() {
  mListMutex.Lock();
  for(lpZGlass_i i=mGlasses.begin(); i!=mGlasses.end(); ++i)
    (*i)->DecRefCount();
  mListMutex.Unlock();
}

/**************************************************************************/

void ZList::Copy(lpZGlass_t& dest)
{
  mListMutex.Lock();
  copy(mGlasses.begin(), mGlasses.end(), back_inserter(dest));
  mListMutex.Unlock();
}

ZGlass* ZList::First()
{
  mListMutex.Lock();
  ZGlass* r = mSize ? mGlasses.front() : 0;
  mListMutex.Unlock();
  return r;
}

ZGlass* ZList::Last()
{ 
  mListMutex.Lock();
  ZGlass* r = mSize ? mGlasses.back() : 0;
  mListMutex.Unlock();
  return r;
}

ZGlass* ZList::GetByName(const Text_t* name)
{
  ZGlass* ret = 0;
  mListMutex.Lock();
  for(lpZGlass_i i=mGlasses.begin(); i!=mGlasses.end(); ++i) {
    if(strcmp(name, (*i)->GetName()) == 0) {
      ret = *i;
      break;
    }
  }
  mListMutex.Unlock();
  return ret;
}

/**************************************************************************/

void ZList::Add(ZGlass* g)
{
  mListMutex.Lock();
  mGlasses.push_back(g); ++mSize;
  StampListAdd(g, 0);
  mListMutex.Unlock();
  g->IncRefCount();
}

void ZList::AddBefore(ZGlass* g, ZGlass* before)
{
  mListMutex.Lock();
  lpZGlass_i i = find(mGlasses.begin(), mGlasses.end(), before);
  mGlasses.insert(i, g); ++mSize;
  StampListAdd(g, before);
  mListMutex.Unlock();
  g->IncRefCount();
}

void ZList::AddFirst(ZGlass* g)
{
  mListMutex.Lock();
  ZGlass* b4 = mSize > 0 ? mGlasses.front() : 0;
  mGlasses.push_front(g); ++mSize;
  StampListAdd(g, b4);
  mListMutex.Unlock();
  g->IncRefCount();
}

void ZList::Remove(ZGlass* g)
{
  if(mSize==0) return;
  mListMutex.Lock();
  lpZGlass_i i = find(mGlasses.begin(), mGlasses.end(), g);
  bool succ;
  if(succ = (i!=mGlasses.end())) {
    mGlasses.erase(i); --mSize;
    StampListRemove(g);
  }
  mListMutex.Unlock();
  if(succ) g->DecRefCount();
}

void ZList::RemoveLast(ZGlass* g)
{
  if(mSize==0) return;
  mListMutex.Lock();
  bool succ = false;
  lpZGlass_i i = mGlasses.end();
  do {
    if(*(--i) == g) {
      mGlasses.erase(i); --mSize;
      StampListRemove(g);
      succ = true;
      break;
    }
  } while(i!=mGlasses.begin());
  mListMutex.Unlock();
  if(succ) g->DecRefCount();
}

/**************************************************************************/

bool ZList::Has(ZGlass* g)
{
  mListMutex.Lock();
  lpZGlass_i i = find(mGlasses.begin(), mGlasses.end(), g);
  bool ret = (i != mGlasses.end());
  mListMutex.Unlock();
  return ret;
}

/**************************************************************************/

// !! The analogous calls in ZGlass have lid/cid counterparts.
// Didn't need them for lists so far. And don't see why I would.
// Just slightly non-consistent.

TimeStamp_t ZList::StampListAdd(ZGlass* g, ZGlass* b4)
{
  if(mSaturn) {
    Ray r(Ray::RQN_list_add, this, g, b4);
    mTimeStamp = mSaturn->Shine(r);
    return mTimeStamp;
  } else {
    if(mStampListAdd_CB)
      mStampListAdd_CB(this, g, b4, mStampListAdd_CBarg);
    return 0;
  }
}

TimeStamp_t ZList::StampListRemove(ZGlass* g)
{
  if(mSaturn) {
    Ray r(Ray::RQN_list_remove, this, g);
    mTimeStamp = mSaturn->Shine(r);
    return mTimeStamp;
  } else {
    if(mStampListRemove_CB)
      mStampListRemove_CB(this, g, mStampListRemove_CBarg);
    return 0;
  }
}

TimeStamp_t ZList::StampListRebuild()
{
  if(mSaturn) {
    Ray r(Ray::RQN_list_rebuild, this);
    mTimeStamp = mSaturn->Shine(r);
    return mTimeStamp;
  } else {
    if(mStampListRebuild_CB)
      mStampListRebuild_CB(this, mStampListRebuild_CBarg);
    return 0;
  }
}

void ZList::SetStampListAdd_CB(zlist_stampadd_f foo, void* arg)
{
  mStampListAdd_CB = foo; mStampListAdd_CBarg = arg;
}
void ZList::SetStampListRemove_CB(zlist_stampremove_f foo, void* arg)
{
  mStampListRemove_CB = foo; mStampListRemove_CBarg = arg;
}
void ZList::SetStampListRebuild_CB(zlist_stamprebuild_f foo, void* arg)
{
  mStampListRebuild_CB = foo; mStampListRebuild_CBarg = arg;
}

/**************************************************************************/

void ZList::Streamer(TBuffer &b)
{
  ZGlass::Streamer(b);
  if(b.IsReading()) {
    mIDs.clear();
    ID_t id;
    b >> mSize;
    ISdebug(D_STREAM, GForm("ZList::Streamer reading %d elements", mSize));
    for(UInt_t i=0; i<mSize; i++) { b >> id; mIDs.push_back(id); }
  } else {
    b << mSize;
    ISdebug(D_STREAM, GForm("ZList::Streamer writing %d elements", mSize));
    for(lpZGlass_i i=mGlasses.begin(); i!=mGlasses.end(); i++)
      b << (*i)->GetSaturnID();
  }
}

Int_t ZList::RebuildList(ZComet* c)
{
  Int_t ret = 0;
  mGlasses.clear();
  for(lID_i i=mIDs.begin(); i!=mIDs.end(); ++i) {
    ZGlass* g = c->FindID(*i);
    if(g) {
      mGlasses.push_back(g);
      g->IncRefCount();
    } else {
      ++ret;
    }
  }
  mIDs.clear();
  return ret;
}

/**************************************************************************/

#include "ZList.c7"
