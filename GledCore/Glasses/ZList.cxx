// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.


//______________________________________________________________________
// ZList
//
//

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Need member-lid, member-cid limiters for Add methods. DONE
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
#include "ZList.c7"

#include <Glasses/ZQueen.h>
#include <Net/Ray.h>
#include <Stones/ZComet.h>
#include <Gled/GledNS.h>

#include <algorithm>
#include <iterator>

ClassImp(ZList)

/**************************************************************************/

void ZList::_init()
{
  mSize = 0;
  mLid  = 0; mCid  = 0;
  mStampListAdd_CB = 0;		mStampListAdd_CBarg = 0;
  mStampListRemove_CB = 0;	mStampListRemove_CBarg = 0;
  mStampListRebuild_CB = 0;	mStampListRebuild_CBarg = 0;
  mStampListClear_CB = 0;	mStampListClear_CBarg = 0;
}

/**************************************************************************/

void ZList::reference_all() {
  PARENT_GLASS::reference_all();
  mListMutex.Lock();
  for(lpZGlass_i i=mGlasses.begin(); i!=mGlasses.end(); ++i)
    (*i)->IncRefCount(this);
  mListMutex.Unlock();
}

void ZList::unreference_all() {
  PARENT_GLASS::unreference_all();
  mListMutex.Lock();
  for(lpZGlass_i i=mGlasses.begin(); i!=mGlasses.end(); ++i)
    (*i)->DecRefCount(this);
  mListMutex.Unlock();
}

/**************************************************************************/

void ZList::clear_list()
{
  mGlasses.clear();
  mSize = 0;
}

/**************************************************************************/

Int_t ZList::remove_references_to(ZGlass* lens)
{
  Int_t nl = ZGlass::remove_references_to(lens);

  int n = 0;
  mListMutex.Lock();
  for(lpZGlass_i i=mGlasses.begin(); i!=mGlasses.end(); ++i) {
    if(*i == lens) {
      lpZGlass_i j = i; --i;
      mGlasses.erase(j);
      ++n;
    }
  }
  mListMutex.Unlock();
  if(n) {
    if(n == 1) StampListRemove(lens);
    else       StampListRebuild();
  }

  return nl + n;
}


/**************************************************************************/

void ZList::new_element_check(ZGlass* g)
{
  static const string _eh("ZList::new_element_check ");

  if(g == 0) {
    throw(_eh + "called with null ZGlass*.");
  }
  if(mLid && mCid) {
    if(!GledNS::IsA(g, FID_t(mLid, mCid))) {
      throw(_eh + "lens of wrong FID_t.");
    }
  }
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

ZGlass* ZList::GetElementByName(const Text_t* name)
{ return GetElementByName(string(name)); }

ZGlass* ZList::GetElementByName(const string& name)
{
  ZGlass* ret = 0;
  mListMutex.Lock();
  for(lpZGlass_i i=mGlasses.begin(); i!=mGlasses.end(); ++i) {
    if(strcmp(name.c_str(), (*i)->GetName()) == 0) {
      ret = *i;
      break;
    }
  }
  mListMutex.Unlock();
  return ret;
}

/**************************************************************************/
/**************************************************************************/

void ZList::SetElementFID(FID_t fid)
{
  mLid = fid.lid; mCid = fid.cid;
  Stamp(FID());
}

void ZList::Add(ZGlass* g)
{
  new_element_check(g);
  mListMutex.Lock();
  mGlasses.push_back(g); ++mSize;
  StampListAdd(g, 0);
  mListMutex.Unlock();
  g->IncRefCount(this);
}

void ZList::AddBefore(ZGlass* g, ZGlass* before)
{
  new_element_check(g);
  mListMutex.Lock();
  lpZGlass_i i = find(mGlasses.begin(), mGlasses.end(), before);
  mGlasses.insert(i, g); ++mSize;
  StampListAdd(g, before);
  mListMutex.Unlock();
  g->IncRefCount(this);
}

void ZList::AddFirst(ZGlass* g)
{
  new_element_check(g);
  mListMutex.Lock();
  ZGlass* b4 = mSize > 0 ? mGlasses.front() : 0;
  mGlasses.push_front(g); ++mSize;
  StampListAdd(g, b4);
  mListMutex.Unlock();
  g->IncRefCount(this);
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
  if(succ) g->DecRefCount(this);
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
  if(succ) g->DecRefCount(this);
}

/**************************************************************************/

void ZList::ClearList()
{
  static const string _eh("ZList::ClearList ");

  if(mSize == 0) return;

  lpZGlass_t foo;
  ISdebug(1, _eh + GForm("locking list '%s'.", GetName()));
  mListMutex.Lock();
  foo.swap(mGlasses);
  clear_list();
  StampListClear();
  mListMutex.Unlock();
  ISdebug(1, _eh + GForm("unlocked list '%s'.", GetName()));
  for(lpZGlass_i i=foo.begin(); i!=foo.end(); ++i) {
    (*i)->DecRefCount(this);
  }
  ISdebug(1, _eh + GForm("sfinished for '%s'.", GetName()));
}

void ZList::ClearAllReferences()
{
  PARENT_GLASS::ClearAllReferences();
  ClearList();
}

/**************************************************************************/

void ZList::RemoveLensesViaQueen(Bool_t recurse)
{
  // Sends MIR to queen and waits for result.
  // This should be called from a detached thread.

  if(IsEmpty()) return;

  auto_ptr<ZMIR> mir( mQueen->S_RemoveLenses(this, recurse) );
  auto_ptr<ZMIR_RR> res( mSaturn->ShootMIRWaitResult(mir) );
  if(res->HasException())
    throw(string(res->Exception.Data()));
}

/**************************************************************************/

Bool_t ZList::Has(ZGlass* g)
{
  mListMutex.Lock();
  lpZGlass_i i = find(mGlasses.begin(), mGlasses.end(), g);
  bool ret = (i != mGlasses.end());
  mListMutex.Unlock();
  return ret;
}

/**************************************************************************/

void ZList::SortByName()
{
  GMutexHolder llck(mListMutex);
  if(mSize < 2) return;
  multimap<string, ZGlass*> nmap;
  for(lpZGlass_i i=mGlasses.begin(); i!=mGlasses.end(); ++i) {
    nmap.insert(pair<string, ZGlass*>((*i)->GetName(), *i));
  }
  mGlasses.clear();
  for(multimap<string, ZGlass*>::iterator i=nmap.begin(); i!=nmap.end(); ++i) {
    mGlasses.push_back(i->second);
  }
  StampListRebuild();
}

/**************************************************************************/
/**************************************************************************/

// !! The analogous calls in ZGlass have lid/cid counterparts.
// Didn't need them for lists so far. And don't see why I would.
// Just slightly non-consistent.

TimeStamp_t ZList::StampListAdd(ZGlass* g, ZGlass* b4)
{
  ++mTimeStamp;
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, RayNS::RQN_list_add, mTimeStamp, Ray::EB_StructuralChange));
    ray->SetBeta(g);
    ray->SetGamma(b4);
    mQueen->EmitRay(ray);
  }
  if(mStampListAdd_CB)
    mStampListAdd_CB(this, g, b4, mStampListAdd_CBarg);

  return mTimeStamp;
}

TimeStamp_t ZList::StampListRemove(ZGlass* g)
{
  ++mTimeStamp;
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, RayNS::RQN_list_remove, mTimeStamp, Ray::EB_StructuralChange));
    ray->SetBeta(g);
    mQueen->EmitRay(ray);
  }
  if(mStampListRemove_CB)
    mStampListRemove_CB(this, g, mStampListRemove_CBarg);

  return mTimeStamp;
}

TimeStamp_t ZList::StampListRebuild()
{
  ++mTimeStamp;
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, RayNS::RQN_list_rebuild, mTimeStamp, Ray::EB_StructuralChange));
    mQueen->EmitRay(ray);
  }
  if(mStampListRebuild_CB)
    mStampListRebuild_CB(this, mStampListRebuild_CBarg);

  return mTimeStamp;
}

TimeStamp_t ZList::StampListClear()
{
  ++mTimeStamp;
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, RayNS::RQN_list_clear, mTimeStamp, Ray::EB_StructuralChange));
    mQueen->EmitRay(ray);
  }
  if(mStampListClear_CB)
    mStampListClear_CB(this, mStampListClear_CBarg);

  return mTimeStamp;
}

/**************************************************************************/

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
void ZList::SetStampListClear_CB(zlist_stampclear_f foo, void* arg)
{
  mStampListClear_CB = foo; mStampListClear_CBarg = arg;
}

/**************************************************************************/

void ZList::Streamer(TBuffer &b)
{
  UInt_t R__s, R__c;

  if(b.IsReading()) {

    Version_t v = b.ReadVersion(&R__s, &R__c);
    ZGlass::Streamer(b);
    b >> mSize >> mLid >> mCid;
    ISdebug(D_STREAM, GForm("ZList::Streamer reading %d elements (%d,%d).",
			    mSize, mLid, mCid));
    mIDs.clear();
    ID_t id;
    for(UInt_t i=0; i<mSize; i++) { b >> id; mIDs.push_back(id); }
    b.CheckByteCount(R__s, R__c, ZList::IsA());

  } else {

    R__c = b.WriteVersion(ZList::IsA(), kTRUE);
    ZGlass::Streamer(b);
    b << mSize << mLid << mCid;
    ISdebug(D_STREAM, GForm("ZList::Streamer writing %d elements (%d,%d).",
			    mSize, mLid, mCid));
    for(lpZGlass_i i=mGlasses.begin(); i!=mGlasses.end(); i++)
      b << (*i)->GetSaturnID();
    b.SetByteCount(R__c, kTRUE);

  }
}

/**************************************************************************/

Int_t ZList::RebuildAllRefs(An_ID_Demangler* idd)
{
  return RebuildLinkRefs(idd) + RebuildListRefs(idd);
}

Int_t ZList::RebuildListRefs(An_ID_Demangler* idd)
{
  Int_t ret = 0;
  mGlasses.clear(); mSize = 0;
  for(lID_i i=mIDs.begin(); i!=mIDs.end(); ++i) {
    ZGlass* lens = idd->DemangleID(*i);
    if(lens) {
      try {
	lens->IncRefCount(this);
	mGlasses.push_back(lens); ++mSize;
      }
      catch(...) {
	++ret;
      }
    } else {
      ++ret;
    }
  }
  mIDs.clear();
  return ret;
}

/**************************************************************************/
