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

typedef pair<TString,lpZGlass_i>           Name2LIter_p;

ClassImp(ZNameMap)

/**************************************************************************/

void ZNameMap::_init()
{
  bReplaceOnInsert = true;
  bReplaceOnRename = false;
}

/**************************************************************************/

void ZNameMap::new_element_check(ZGlass* lens)
{
  static const Exc_t _eh("ZNameMap::new_element_check ");

  PARENT_GLASS::new_element_check(lens);
  mName2Iter_i i = mItMap.find(lens->StrName());
  if(i != mItMap.end()) {
    if(bReplaceOnInsert) {
      ZList::iterator l = i->second;
      mItMap.erase(i);
      l.lens()->unregister_name_change_cb(this);
      mElements.erase(l); --mSize;
      l.lens()->DecRefCount(this);
      StampListRemove(l->fId);
    } else {
      throw(_eh + "lens " + lens->Identify() + " already in the list.");
    }
  }
}

void ZNameMap::clear_list()
{
  for(ZList::iterator i=begin(); i!=end(); ++i)
    i.lens()->unregister_name_change_cb(this);
  PARENT_GLASS::clear_list();
  mItMap.clear();
}

/**************************************************************************/

void ZNameMap::on_insert(ZList::iterator it)
{
  it.lens()->register_name_change_cb(this);
  mItMap[it.lens()->GetName()] = it;
}

void ZNameMap::on_remove(ZList::iterator it)
{
  static const Exc_t _eh("ZNameMap::on_remove ");

  mName2Iter_i i = mItMap.find(it.lens()->StrName());
  if(i == mItMap.end()) {
    ISwarn(_eh + "*strange*, name not found in the map.");
    return;
  }
  if(i->second != it) {
    ISwarn(_eh + "*strange*, got uncompatible iterators.");
  }
  mItMap.erase(i);
  it.lens()->unregister_name_change_cb(this);
}

void ZNameMap::on_rebuild()
{
  mItMap.clear();
  for(ZList::iterator i=begin(); i!=end(); ++i) {
    i.lens()->register_name_change_cb(this);
    mItMap[i.lens()->StrName()] = i;
  }
}

/**************************************************************************/

ZGlass* ZNameMap::GetElementByName(const TString& name)
{
  GMutexHolder lck(mListMutex);
  mName2Iter_i i = mItMap.find(name);
  return (i != mItMap.end()) ? i->second.lens() : 0;
}

/**************************************************************************/

void ZNameMap::SortByName()
{

  GMutexHolder llck(mListMutex);
  if(mSize < 2) return;

  list<element> store;
  mElements.swap(store);
  mNextId = 0;
  for(mName2Iter_i i=mItMap.begin(); i!=mItMap.end(); ++i) {
    mElements.push_back(element(i->second.lens(), mNextId++));
    i->second = --mElements.end();
  }
  on_rebuild();
  StampListRebuild();
}

/**************************************************************************/

void ZNameMap::name_change_cb(ZGlass* g, const TString& new_name)
{
  static const Exc_t _eh("ZNameMap::name_change_cb ");

  TString old_name(g->StrName());
  if(new_name == old_name) return;

  GMutexHolder list_lock(mListMutex);

  mName2Iter_i o = mItMap.find(old_name);
  if(o == mItMap.end()) {
    throw(_eh + "element " + g->Identify() + " not found in map " + Identify() + ".");
  }
  mName2Iter_i n = mItMap.find(new_name);
  if(n != mItMap.end()) {
    if(bReplaceOnRename) { // Remove the old one (with new name).
      ISwarn(_eh + "removing conflicting element " + n->second.lens()->Identify() +
	     "as another element changed its name to the same value..");
      // remove at n
      ZList::iterator l    = n->second;
      ZGlass*         lens = l.lens();
      mItMap.erase(n);
      lens->unregister_name_change_cb(this);
      mElements.erase(l); --mSize;
      lens->DecRefCount(this);
      // remove o and recreate with n
      mItMap[new_name] = o->second;
      mItMap.erase(o);
      StampListRemove(l->fId);
    } else {
      ISwarn(_eh + "removing element " + g->Identify() +
	     " that changed its name to the value already existing in the map.");
      // remove at o
      ZList::iterator l    = o->second;
      ZGlass*         lens = l.lens();
      mItMap.erase(o);
      lens->unregister_name_change_cb(this);
      mElements.erase(l); --mSize;
      lens->DecRefCount(this);
      StampListRemove(l->fId);
    }
  }
}
