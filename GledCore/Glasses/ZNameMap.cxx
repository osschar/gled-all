// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZNameMap
//
// Keeps elements sorted by their name. Registers self to each element of
// the map to receive name-change callback.
// All add methods are overridden to the Insert() method.
//

#include "ZNameMap.h"
#include "ZNameMap.c7"

typedef map<string, lpZGlass_i>	          Name2LIter_t;
typedef map<string, lpZGlass_i>::iterator Name2LIter_i;
typedef pair<string,lpZGlass_i>           Name2LIter_p;

ClassImp(ZNameMap)

/**************************************************************************/

void ZNameMap::_init()
{}

/**************************************************************************/

ZGlass* ZNameMap::GetByName(const Text_t* name)
{
  mListMutex.Lock();
  Name2LIter_i i = mItMap.find(name);
  ZGlass* ret = (i != mItMap.end()) ? *(i->second) : 0;
  mListMutex.Unlock();
  return ret;
}

/**************************************************************************/

lpZGlass_i ZNameMap::insert(ZGlass* g, const string& name)
{
  pair<Name2LIter_i,bool> res = mItMap.insert(Name2LIter_p(name,0));
  if(res.second) {
    Name2LIter_i x = res.first; ++x;
    lpZGlass_i b4 = (x != mItMap.end()) ? x->second : mGlasses.end();
    res.first->second = mGlasses.insert(b4, g);
    return b4;
  } else {
    throw(string("ZNameMap::insert insertion failed (name already in the map)"));
  }
}

void ZNameMap::Insert(ZGlass* g)
{
  new_element_check(g);
  string name(g->GetName());
  mListMutex.Lock();
  try {
    lpZGlass_i before = insert(g, name);
    ++mSize;
    StampListAdd(g, (before != mGlasses.end() ? *before : 0));
    mListMutex.Unlock();
  }
  catch(string exc) {
    mListMutex.Unlock();
    throw;
  }
  g->IncRefCount(this);
  g->register_name_change_cb(this);
}

void ZNameMap::Add(ZGlass* g)
{ Insert(g); }

void ZNameMap::AddBefore(ZGlass* g, ZGlass* before)
{ Insert(g); }

void ZNameMap::AddFirst(ZGlass* g)
{ Insert(g); }

void ZNameMap::Remove(ZGlass* g)
{
  // Removes a lens g from the map.

  mListMutex.Lock();
  Name2LIter_i i = mItMap.find(g->GetName());
  if(i != mItMap.end()) {
    mGlasses.erase(i->second); --mSize;
    mItMap.erase(i);
    StampListRemove(g);
    mListMutex.Unlock();
    g->DecRefCount(this);
    g->register_name_change_cb(this);
  } else {
    // Try also "remove by glass ptr"
    UInt_t old_size = mSize;
    PARENT_GLASS::Remove(g);
    if(old_size != mSize) {
      RecreateMap();
      StampListRebuild();
      ISwarn(GForm("ZNameMap::Remove rebuilt list contents"));
    }
    mListMutex.Unlock();
  }
}

void ZNameMap::RemoveLast(ZGlass* g)
{ Remove(g); }

void ZNameMap::Clear()
{
  PARENT_GLASS::Clear();
  mItMap.clear();
}

/**************************************************************************/

void ZNameMap::RecreateMap()
{
  mListMutex.Lock();
  map<string,ZGlass*> buf;
  for(lpZGlass_i i=mGlasses.begin(); i!=mGlasses.end(); ++i) {
    pair<map<string,ZGlass*>::iterator, bool> res =
      buf.insert(pair<string,ZGlass*>((*i)->GetName(), *i));
    if(res.second == false) 
      (*i)->DecRefCount(this);
  }
  mItMap.clear(); mGlasses.clear(); mSize = 0;
  for(map<string,ZGlass*>::iterator i=buf.begin(); i!=buf.end(); ++i) {
    mGlasses.push_back(i->second); ++mSize;
    lpZGlass_i j = mGlasses.end(); --j;
    mItMap.insert(Name2LIter_p(i->first, j));
  }
  mListMutex.Unlock();
}

/**************************************************************************/

Int_t ZNameMap::RebuildList(ZComet* c)
{
  Int_t ret = ZList::RebuildList(c);
  mItMap.clear();
  for(lpZGlass_i i=mGlasses.begin(); i!=mGlasses.end(); ++i) {
    mItMap.insert(Name2LIter_p((*i)->GetName(), i));
    (*i)->register_name_change_cb(this);
  }
  return ret;
}

/**************************************************************************/

void ZNameMap::y_name_change_cb(ZGlass* g, const string& new_name)
{
  string old_name(g->GetName());
  if(new_name == old_name) return;
  mListMutex.Lock();
  Name2LIter_i n = mItMap.find(new_name);
  if(n != mItMap.end()) {
    ISwarn("ZNameMap::Y_name_change_cb removing element that changed its name to the value already existing in the map");
    Remove(g);
  } else {
    Name2LIter_i i = mItMap.find(old_name);
    if(i != mItMap.end()) {
      lpZGlass_i b4 = insert(g, new_name);
      bool stamp_p = true;
      if(b4 == i->second || --(--b4) == i->second)
	stamp_p = false;
      mGlasses.erase(i->second);
      mItMap.erase(i);	
      if(stamp_p)
	StampListRebuild();
    } else {
      mListMutex.Unlock();
      throw(string("ZNameMap::Y_name_change_cb element not found in map"));
    }
  }
  mListMutex.Unlock();
}
