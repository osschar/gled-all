// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZStringMap
//
//

#include "ZStringMap.h"
#include "ZStringMap.c7"

ClassImp(ZStringMap);

/**************************************************************************/

void ZStringMap::_init()
{
  mAddLensPrefix = "undef-";
  mAddLensFormat = "%03d";
  mAddLensCurId  = 1;
}

/**************************************************************************/
// AList methods, protected part.
/**************************************************************************/

Int_t ZStringMap::remove_references_to(ZGlass* lens)
{
  Int_t n  = ZGlass::remove_references_to(lens);
  GMutexHolder llck(mListMutex);
  iterator i = begin();
  while(i != end()) {
    if(i() == lens) {
      iterator j = i++;
      TString label = j->first;
      on_remove(j);
      mElements.erase(j);
      StampListRemoveLabel(lens, label);
      ++n;
    } else {
      ++i;
    }
  }
  return n;
}

void ZStringMap::clear_list()
{
  mSize = 0;
  mElements.clear();
}

/**************************************************************************/
// AList methods, public part.
/**************************************************************************/

Int_t ZStringMap::RebuildListRefs(An_ID_Demangler* idd)
{
  Int_t ret = 0;
  iterator i = begin();
  while(i != end()) {
    ZGlass* lens = idd->DemangleID(GledNS::CastLens2ID(i->second));
    bool ok = false;
    if(lens) {
      try {
	lens->IncRefCount(this);
        i->second = lens;
        ok = true;
        ++i;
      }
      catch(Exc_t& exc) {
        // !!! warn
      }
    }
    if(!ok) {
      iterator j = i++;
      mElements.erase(j);
      --mSize;
      ++ret;
    }
  }
  on_rebuild();
  return ret;
}

void ZStringMap::ClearList()
{
  static const Exc_t _eh("ZStringMap::ClearList ");

  if(mSize == 0) return;

  container foo;
  ISdebug(0, _eh + GForm("locking list '%s'.", GetName()));
  mListMutex.Lock();
  foo.swap(mElements);
  clear_list();
  StampListClear();
  mListMutex.Unlock();
  ISdebug(0, _eh + GForm("unlocked list '%s'.", GetName()));
  for(iterator i=foo.begin(); i!=foo.end(); ++i) {
    i()->DecRefCount(this);
  }
  ISdebug(0, _eh + GForm("finished for '%s'.", GetName()));
}

AList::stepper_base* ZStringMap::make_stepper_imp(bool return_zeros)
{
  return new stepper_imp<ZStringMap>(begin(), end(), return_zeros);
}

/**************************************************************************/
// Generick AList
/**************************************************************************/

void ZStringMap::Add(ZGlass* lens)
{
  // Adds lens with the unique 'unknown' label. Label string is
  // specified by members mAddLensPrefix and mAddLensFormat.

  GMutexHolder llck(mListMutex);
  new_element_check(lens);
  TString  label;
  iterator i;
  do {
    label  = mAddLensPrefix;
    label += GForm(mAddLensFormat, mAddLensCurId++);
    i = mElements.find(label);
  } while(i != end());
  lens->IncRefCount(this);
  pair<iterator, bool> r = mElements.insert(element(label, lens));
  ++mSize;
  i = r.first;
  on_insert(i);
  ++i;
  StampListInsertLabel(lens, label, i != end() ? i->first : "");
}

void ZStringMap::RemoveAll(ZGlass* lens)
{
  Int_t n=0;
  GMutexHolder llck(mListMutex);
  iterator i = begin();
  while(i != end()) {
    if(i() == lens) {
      iterator j = i++;
      StampListRemoveLabel(j->second, j->first);
      mElements.erase(j); --mSize;
      ++n;
    } else {
      ++i;
    }
  }
  if(n) lens->DecRefCount(this, n);
  mListMutex.Unlock();
}

/**************************************************************************/
// Label operations
/**************************************************************************/

void ZStringMap::SetElementByLabel(ZGlass* lens, TString label)
{
  static const Exc_t _eh("ZStringMap::SetElementByLabel ");

  if(label.IsNull())
    throw(_eh + "argument label is null.");

  GMutexHolder llck(mListMutex);
  new_element_check(lens);

  iterator i = mElements.find(label);
  if(i != end()) {
    if(i->second == lens)
      return;
    ZGlass* exlens = i->second;
    if(lens) lens->IncRefCount(this);
    i->second = lens;
    if(exlens) exlens->DecRefCount(this);
    on_change(i, exlens);
    StampListElementSet(lens, label);
  } else {
    if(lens) lens->IncRefCount(this);
    pair<iterator, bool> r = mElements.insert(element(label, lens));
    ++mSize;
    i = r.first;
    on_insert(i);
    ++i;
    StampListInsertLabel(lens, label, i != end() ? i->first : "");
  }
}

void ZStringMap::AddLabel(TString label)
{
  static const Exc_t _eh("ZStringMap::AddLabel ");

  if(label.IsNull())
    throw(_eh + "argument label is null.");

  GMutexHolder llck(mListMutex);

  if(label.IsNull())
    throw(_eh + "null label supplied,");
  iterator i = mElements.find(label);
  if(i != end())
    throw(_eh + "label '" + label + "' already exists.");

  pair<iterator, bool> r = mElements.insert(element(label, 0));
  ++mSize;
  i = r.first;
  on_insert(i);
  ++i;
  StampListInsertLabel(0, label, i != end() ? i->first : "");

}

void ZStringMap::RemoveLabel(TString label)
{
  static const Exc_t _eh("ZStringMap::RemoveLabel ");

  if(label.IsNull())
    throw(_eh + "argument label is null.");

  GMutexHolder llck(mListMutex);

  iterator i = mElements.find(label);
  if(i == end())
    throw(_eh + "label '" + label + "' not found.");

  ZGlass* lens = i->second;
  on_remove(i);
  mElements.erase(i); --mSize;
  lens->DecRefCount(this);
  StampListRemoveLabel(lens, label);
}

void ZStringMap::ChangeLabel(TString label, TString new_label)
{
  // Renames label to new_label.
  // It is an error if new_label already exists.
  //
  // If successful it emits two rays: RemoveLabel and InsertLabel.

  static const Exc_t _eh("ZStringMap::ChangeLabel ");

  if(label.IsNull())
    throw(_eh + "argument label is null.");
  if(new_label.IsNull())
    throw(_eh + "argument new_label is null.");

  GMutexHolder llck(mListMutex);

  iterator i;
  i = mElements.find(new_label);
  if(i != end())
    throw(_eh + "new_label '" + new_label + "' already exists.");
  i = mElements.find(label);
  if(i == end())
    throw(_eh + "label '" + label + "' not found.");

  ZGlass* lens = i->second;
  mElements.erase(i);
  StampListRemoveLabel(lens, label);
  pair<iterator, bool> r = mElements.insert(element(new_label, lens));
  i = r.first;
  on_change_label(i, label);
  ++i;
  StampListInsertLabel(lens, new_label, i != end() ? i->first : "");
}

/**************************************************************************/
/**************************************************************************/
// Streamer
/**************************************************************************/

void ZStringMap::Streamer(TBuffer &b)
{
  UInt_t R__s, R__c;

  if(b.IsReading()) {

    Version_t R__v = b.ReadVersion(&R__s, &R__c); if(R__v) { }
    AList::Streamer(b);
    b >> mAddLensPrefix >> mAddLensFormat >> mAddLensCurId;
    TString str;
    ID_t    id;
    mElements.clear();
    container::iterator p = mElements.begin();
    for(Int_t i=0; i<mSize; ++i) {
      b >> id >> str;
      p = mElements.insert(p, element(str, (ZGlass*)id));
    }
    b.CheckByteCount(R__s, R__c, ZStringMap::IsA());

  } else {

    R__c = b.WriteVersion(ZStringMap::IsA(), kTRUE);
    AList::Streamer(b);
    b << mAddLensPrefix << mAddLensFormat << mAddLensCurId;
    for(iterator i=begin(); i!=end(); ++i)
      b << i()->GetSaturnID() << i->first;
    b.SetByteCount(R__c, kTRUE);

  }
}

/**************************************************************************/
