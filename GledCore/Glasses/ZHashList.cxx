// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZHashList
//
//

#include "ZHashList.h"
#include "ZHashList.c7"

typedef hash_map<ZGlass*, lpZGlass_i>		Glass2LIter_t;
typedef hash_map<ZGlass*, lpZGlass_i>::iterator	Glass2LIter_i;

ClassImp(ZHashList)

/**************************************************************************/

void ZHashList::_init()
{
  bNerdyListOps = true;
}

/**************************************************************************/

void ZHashList::clear_list()
{
  PARENT_GLASS::clear_list();
  mItHash.clear();
}


/**************************************************************************/

Int_t ZHashList::remove_references_to(ZGlass* lens)
{
  Int_t n = ZGlass::remove_references_to(lens);
  mListMutex.Lock();
  Glass2LIter_i i = mItHash.find(lens);
  if(i != mItHash.end()) {
    mGlasses.erase(i->second); --mSize;
    mItHash.erase(i);
    StampListRemove(lens);
    ++n;
  }
  mListMutex.Unlock();

  return n;
}

/**************************************************************************/

void ZHashList::rebuild_hash()
{
  mItHash.clear();
  for(lpZGlass_i i=mGlasses.begin(); i!=mGlasses.end(); ++i) {
    mItHash[*i] = i;
  }
}

/**************************************************************************/

void ZHashList::Add(ZGlass* g)
{
  new_element_check(g);
  if(!Has(g)) {
    mListMutex.Lock();
    mGlasses.push_back(g); ++mSize;
    mItHash[g] = --mGlasses.end();
    StampListAdd(g, 0);
    mListMutex.Unlock();
    g->IncRefCount(this);
  } else {
    if(bNerdyListOps)
      throw(string("ZHashList::Add element already in the list"));
  }
}

void ZHashList::AddBefore(ZGlass* g, ZGlass* before)
{
  new_element_check(g);
  if(!Has(g)) {
    mListMutex.Lock();
    Glass2LIter_i h = mItHash.find(before);
    lpZGlass_i i = (h != mItHash.end()) ? h->second : mGlasses.end();
    mGlasses.insert(i, g); ++mSize;
    mItHash[g] = --i;
    StampListAdd(g, before);
    mListMutex.Unlock();
    g->IncRefCount(this);
  } else {
    if(bNerdyListOps)
      throw(string("ZHashList::AddBefore element already in the list"));
  }
}

void ZHashList::AddFirst(ZGlass* g)
{
  new_element_check(g);
  if(!Has(g)) {
    mListMutex.Lock();
    ZGlass* b4 = mSize > 0 ? mGlasses.front() : 0;
    mGlasses.push_front(g); ++mSize;
    mItHash[g] = mGlasses.begin();
    StampListAdd(g, b4);
    mListMutex.Unlock();
    g->IncRefCount(this);
  } else {
    if(bNerdyListOps)
      throw(string("ZHashList::AddFirst element already in the list"));
  }
}

void ZHashList::Remove(ZGlass* g)
{
  mListMutex.Lock();
  Glass2LIter_i i = mItHash.find(g);
  if(i != mItHash.end()) {
    mGlasses.erase(i->second); --mSize;
    mItHash.erase(i);
    StampListRemove(g);
    g->DecRefCount(this);
  } else {
    if(bNerdyListOps) {
      mListMutex.Unlock();
      throw(string("ZHashList::Remove element not in the list"));
    }
  }
  mListMutex.Unlock();
}

void ZHashList::RemoveLast(ZGlass* g)
{ Remove(g); }

/**************************************************************************/

Bool_t ZHashList::Has(ZGlass* g)
{
  mListMutex.Lock();
  Glass2LIter_i i = mItHash.find(g);
  bool ret = (i != mItHash.end());
  mListMutex.Unlock();
  return ret;
}

/**************************************************************************/

ZGlass* ZHashList::After(ZGlass* g)
{
  ZGlass* ret = 0;
  mListMutex.Lock();
  Glass2LIter_i i = mItHash.find(g);
  if(i != mItHash.end()) {
    lpZGlass_i j(i->second); ++j;
    if(j != mGlasses.end())
      ret = *j;
  }
  mListMutex.Unlock();
  return ret;
}

ZGlass* ZHashList::Before(ZGlass* g)
{
  ZGlass* ret = 0;
  mListMutex.Lock();
  Glass2LIter_i i = mItHash.find(g);
  if(i != mItHash.end()) {
    lpZGlass_i j(i->second);
    if(j != mGlasses.begin())
      ret = *(--j);
  }
  mListMutex.Unlock();
  return ret;
}

/**************************************************************************/

void ZHashList::SortByName()
{
  GMutexHolder llck(mListMutex);
  ZList::SortByName();
  rebuild_hash();
}

/**************************************************************************/

Int_t ZHashList::RebuildListRefs(An_ID_Demangler* idd)
{
  Int_t ret = ZList::RebuildListRefs(idd);
  rebuild_hash();
  return ret;
}

/**************************************************************************/
