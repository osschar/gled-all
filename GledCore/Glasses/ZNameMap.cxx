// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
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
{
  bReplaceOnInsert = true;
  bReplaceOnRename = false;
}

/**************************************************************************/

void ZNameMap::clear_list()
{
  PARENT_GLASS::clear_list();
  mItMap.clear();
}

/**************************************************************************/

Int_t ZNameMap::remove_references_to(ZGlass* lens)
{
  Int_t n = ZGlass::remove_references_to(lens);
  mListMutex.Lock();
  Name2LIter_i i = mItMap.find(lens->GetName());
  if(i != mItMap.end()) {
    mGlasses.erase(i->second); --mSize;
    mItMap.erase(i);
    StampListRemove(lens);
    lens->unregister_name_change_cb(this);
    ++n;
  }
  mListMutex.Unlock();
  return n;
}

/**************************************************************************/

ZGlass* ZNameMap::GetElementByName(const string& name)
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
  static const string _eh("ZNameMap::insert ");

  Name2LIter_i i = mItMap.find(name);
  if(i != mItMap.end()) {
    if(bReplaceOnInsert) {
      ZGlass* ex_el = *i->second;
      warn_caller(_eh + "replacing element '" + ex_el->Identify() + "'.");
      Remove(ex_el);
    } else {
      throw(_eh + "insertion failed (name already in the map).");
    }
  }

  pair<Name2LIter_i,bool> res = mItMap.insert(Name2LIter_p(name,0));

  Name2LIter_i x = res.first; ++x;
  lpZGlass_i b4 = (x != mItMap.end()) ? x->second : mGlasses.end();
  res.first->second = mGlasses.insert(b4, g);
  ++mSize;

  return b4;
}

void ZNameMap::Insert(ZGlass* g)
{
  new_element_check(g);
  string name(g->GetName());

  GMutexHolder list_lock(mListMutex);

  g->IncRefCount(this);
  lpZGlass_i before;
  try        { before = insert(g, name); }
  catch(...) { g->DecRefCount(this); throw; }
  g->register_name_change_cb(this);

  StampListAdd(g, (before != mGlasses.end() ? *before : 0));
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
    g->unregister_name_change_cb(this);
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

Int_t ZNameMap::RebuildListRefs(An_ID_Demangler* idd)
{
  Int_t ret = ZList::RebuildListRefs(idd);
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
  static const string _eh("ZNameMap::y_name_change_cb ");

  string old_name(g->GetName());
  if(new_name == old_name) return;

  GMutexHolder list_lock(mListMutex);

  Name2LIter_i n = mItMap.find(new_name);
  Name2LIter_i o = mItMap.find(old_name);
  if(o == mItMap.end()) {
    ISwarn(_eh + "element '" + g->Identify() + "'not found in map '" +
	   Identify() + "'.");
    return;
  }

  if(n != mItMap.end()) {
    if(bReplaceOnRename) {
      ISwarn(_eh + "removing element that changed its name to the value already existing in the map.");
      Remove(*n->second);
    } else {
      ISwarn(_eh + "removing element that changed its name to the value already existing in the map.");
      Remove(g);
      return;
    }
  }

  lpZGlass_i before = insert(g, new_name);
  mGlasses.erase(o->second);
  mItMap.erase(o);	

  StampListRemove(g);
  StampListAdd(g, (before != mGlasses.end() ? *before : 0));
}
