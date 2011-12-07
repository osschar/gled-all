// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZNameMap.h"
#include "ZNameMap.c7"

typedef pair<TString,lpZGlass_i>           Name2LIter_p;

//______________________________________________________________________________
//
// Keeps a parallel multi-map with elements sorted by their name.
// This is used both to have faster "by-name" search and to
// allow for lists sorted by name.
//
// Registers itself to each element of the map to receive name-change
// callback. This mechanism is rather flawed - it should be extended
// significanly to become bullet-proof:
//   - name-change and consumer registration should be locked (per queen?);
//   - name-change notifications should be sent as MIRs;
//   - the sun copy should handle all this, moon copies should be updated
//     by special MIRs;
//   - all this implies that name-change registration would only be done at
//     the sun.
// Oh, also, the insertions should be done at the sun with some more specific
// MIR being sent to the moons.
//
// The bKeepSorted flag enforces that the list remains sorted by name
// most of the time (the request is sent via a MIR). This is primarily
// intended for GUI views. This could also be improved with the above changes.

ClassImp(ZNameMap);

//------------------------------------------------------------------------------

void ZNameMap::_init()
{
  bKeepSorted    = false;
  bWarnEqualName = false;
}

ZNameMap::ZNameMap(const Text_t* n, const Text_t* t) : ZList(n,t)
{
  _init();
}

ZNameMap::~ZNameMap()
{}

//------------------------------------------------------------------------------

void ZNameMap::new_element_check(ZGlass* lens)
{
  static const Exc_t _eh("ZNameMap::new_element_check ");

  PARENT_GLASS::new_element_check(lens);

  mName2Iter_i i = mItMap.find(lens->StrName());
  if(i != mItMap.end() && bWarnEqualName) {
    // Could in principle deny request.
    // The problem is I can do no such thing in name_change_cb. Well,
    // one could shoot a mir requesting removal of one or the other entry.
    ISwarn(_eh + "lens with the same name as '" + lens->Identify() + "' already in the map.");
  }
}

void ZNameMap::clear_list()
{
  for(ZList::iterator i=begin(); i!=end(); ++i)
    i.lens()->unregister_name_change_cb(this);
  PARENT_GLASS::clear_list();
  mItMap.clear();
}

//------------------------------------------------------------------------------

void ZNameMap::on_insert(ZList::iterator it)
{
  it.lens()->register_name_change_cb(this);
  mItMap.insert(make_pair(it.lens()->StrName(), it));
}

void ZNameMap::on_remove(ZList::iterator it)
{
  static const Exc_t _eh("ZNameMap::on_remove ");

  mName2Iter_i_pair range = mItMap.equal_range(it.lens()->StrName());

  if(range.first == range.second) {
    ISwarn(_eh + "*strange*, name not found in the map.");
    return;
  }

  for(mName2Iter_i i = range.first; i != range.second; ++i) {
    if(i->second == it) {
      mItMap.erase(i);
      it.lens()->unregister_name_change_cb(this);
      return;
    }
  }

  // If we get here, match was not found.
  ISwarn(_eh + "*strange*, got uncompatible iterators.");
}

void ZNameMap::on_rebuild()
{
  mItMap.clear();
  for(ZList::iterator i=begin(); i!=end(); ++i) {
    mItMap.insert(make_pair(i.lens()->StrName(), i));
  }
}

//------------------------------------------------------------------------------

void ZNameMap::SetKeepSorted(Bool_t keep_sorted)
{
  if(keep_sorted == bKeepSorted)
    return;

  if(keep_sorted)
    SortByName();

  bKeepSorted = keep_sorted;

  Stamp(FID());
}

//------------------------------------------------------------------------------

ZGlass* ZNameMap::GetElementByName(const TString& name)
{
  GMutexHolder lck(mListMutex);
  mName2Iter_i i = mItMap.find(name);
  return (i != mItMap.end()) ? i->second.lens() : 0;
}

Int_t ZNameMap::GetElementsByName(const TString& name, lpZGlass_t& dest)
{
  Int_t n = 0;
  GMutexHolder lck(mListMutex);
  mName2Iter_i_pair range = mItMap.equal_range(name);
  for(mName2Iter_i i=range.first; i!=range.second; ++i) {
    dest.push_back(i->second.lens());
    ++n;
  }
  return n;
}

//------------------------------------------------------------------------------

void ZNameMap::shoot_sort_mir()
{
  if(IsSunSpace()) {
    auto_ptr<ZMIR> mir(S_SortByName());
    mSaturn->ShootMIR(mir); 
  }
}

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

void ZNameMap::DumpNameMap()
{
  static const Exc_t _eh("ZNameMap::DumpNameMap ");

  GMutexHolder list_lock(mListMutex);

  printf("%s %s, mSize=%d\n", _eh.Data(), Identify().Data(), mSize);
  Int_t n = 1;
  for(mName2Iter_i i=mItMap.begin(); i!=mItMap.end(); ++i, ++n)
  {
    printf("%4d '%s'%*c - %s\n", n, i->first.Data(), 20 - i->first.Length(), ' ', i->second()->Identify().Data());
  }
}

//------------------------------------------------------------------------------
// Functions that add elements - bKeepSorted overrides placement
//------------------------------------------------------------------------------

void ZNameMap::insert_with_placement(ZGlass* lens)
{
  static const Exc_t _eh("ZNameMap::insert_with_placement ");

  GMutexHolder llck(mListMutex);
  new_element_check(lens);

  TString name = lens->StrName();

  ZList::iterator i;
  {
    mName2Iter_i nii = mItMap.upper_bound(lens->StrName());
    if (nii == mItMap.end())
      i = mElements.end();
    else
      i = nii->second;
  }
  lens->IncRefCount(this);
  ZList::iterator j = mElements.insert(i, element(lens, mNextId++)); ++mSize;
  on_insert(j);

  if (i == mElements.end())
    StampListPushBack(lens, mNextId-1);
  else
    StampListInsert(lens, mNextId-1, i->fId);
}

void ZNameMap::PushBack(ZGlass* lens)
{
  if (bKeepSorted) {
    insert_with_placement(lens);
  } else {
    ZList::PushBack(lens);
  }
}

void ZNameMap::PushFront(ZGlass* lens)
{
  if (bKeepSorted) {
    insert_with_placement(lens);
  } else {
    ZList::PushFront(lens);
  }
}

void ZNameMap::InsertById(ZGlass* lens, Int_t before_id)
{
  if (bKeepSorted) {
    insert_with_placement(lens);
  } else {
    ZList::InsertById(lens, before_id);
  }
}

//------------------------------------------------------------------------------
// Virtuals from non-glasses
//------------------------------------------------------------------------------

void ZNameMap::name_change_cb(ZGlass* lens, const TString& new_name)
{
  static const Exc_t _eh("ZNameMap::name_change_cb ");

  const TString& old_name(lens->RefName());

  if(new_name == old_name) return;

  GMutexHolder list_lock(mListMutex);

  mName2Iter_i_pair range = mItMap.equal_range(old_name);

  if(range.first == range.second) {
    // This is bad and should NOT happen.
    // No really good way to handle it here ... unless we rescan the whole
    // map and search for the lens via its address.
    ISerr(_eh + "element " + lens->Identify() + " not found in map " + Identify() + ".");
    return;
  }

  if(bWarnEqualName) {
    mName2Iter_i n = mItMap.find(new_name);
    if(n != mItMap.end()) {
      ISwarn(_eh + "element with the same name as '" + lens->Identify() + "' already in the map.");
    }
  }

  list<mName2Iter_pair> newelms;
  for(mName2Iter_i i = range.first; i != range.second;) {
    if(i->second.lens() == lens) {
      newelms.push_back(make_pair(new_name, i->second));
      mName2Iter_i j = i++;
      mItMap.erase(j);
    } else {
      ++i;
    }
  }
  for(list<mName2Iter_pair>::iterator i=newelms.begin(); i!=newelms.end(); ++i)
    mItMap.insert(*i);

  if(bKeepSorted)
    shoot_sort_mir();
}
