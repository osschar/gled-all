// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZDeque
//
//

#include "ZDeque.h"
#include "ZDeque.c7"

#include <algorithm>
#include <iterator>

ClassImp(ZDeque)

/**************************************************************************/

void ZDeque::_init()
{
}

/**************************************************************************/

void ZDeque::clear_list()
{
  mSize = 0;
  mElements.clear();
}

Int_t ZDeque::remove_references_to(ZGlass* lens)
{
  Int_t m  = ZGlass::remove_references_to(lens);
  Int_t n = 0;
  GMutexHolder llck(mListMutex);
  for(iterator i=begin(); i!=end(); ++i) {
    if(i() == lens) {
      iterator j = i; --i;
      on_remove(j);
      mElements.erase(j);
      ++n;
    }
  }
  if(n > 0) {
    StampListRebuild();
  }
  return n + m;
}

/**************************************************************************/
// AList methods, public part.
/**************************************************************************/

AList::stepper_base* ZDeque::make_stepper_imp()
{
  return new stepper_imp<ZDeque>(begin(), end());
}

/**************************************************************************/

Int_t ZDeque::RebuildListRefs(An_ID_Demangler* idd)
{
  Int_t ret = 0;
  container in;
  mElements.swap(in);
  mSize   = 0;
  for(iterator i=in.begin(); i!=in.end(); ++i) {
    ZGlass* lens = idd->DemangleID(GledNS::CastLens2ID(i()));
    if(lens) {
      try {
	lens->IncRefCount(this);
	mElements.push_back(lens); ++mSize;
      }
      catch(...) {
	++ret;
      }
    } else {
      ++ret;
    }
  }
  on_rebuild();
  return ret;
}

void ZDeque::ClearList()
{
  static const Exc_t _eh("ZDeque::ClearList ");

  if(mSize == 0) return;

  container foo;
  ISdebug(1, _eh + GForm("locking list '%s'.", GetName()));
  mListMutex.Lock();
  foo.swap(mElements);
  clear_list();
  StampListClear();
  mListMutex.Unlock();
  ISdebug(1, _eh + GForm("unlocked list '%s'.", GetName()));
  for(iterator i=foo.begin(); i!=foo.end(); ++i) {
    i()->DecRefCount(this);
  }
  ISdebug(1, _eh + GForm("sfinished for '%s'.", GetName()));
}

/**************************************************************************/
// Generick
/**************************************************************************/

void ZDeque::Add(ZGlass* lens)
{
  PushBack(lens);
}

Int_t ZDeque::RemoveAll(ZGlass* lens)
{
  Int_t n  = 0;
  mListMutex.Lock();
  for(iterator i=begin(); i!=end(); ++i) {
    if(i() == lens) {
      iterator j = i; --i;
      mElements.erase(j); --mSize;
      ++n;
    }
  }
  mListMutex.Unlock();
  if(n) {
    lens->DecRefCount(this, n);
    StampListRebuild();
  }
  return n;
}

/**************************************************************************/
// Deque
/**************************************************************************/

ZGlass* ZDeque::FrontElement()
{
  ZGlass* l;
  { GMutexHolder llck(mListMutex);
    l = mSize ? mElements.front() : 0;
  }
  ZMIR* mir = get_MIR();
  if(mir && mir->HasResultReq()) {
    TBufferFile b(TBuffer::kWrite);
    GledNS::WriteLensID(b, l);
    mSaturn->ShootMIRResult(b);
  }
  return l;
}

ZGlass* ZDeque::BackElement()
{
  ZGlass* l;
  { GMutexHolder _lstlck(mListMutex);
    l = mSize ? mElements.back() : 0;
  }
  ZMIR* mir = get_MIR();
  if(mir && mir->HasResultReq()) {
    TBufferFile b(TBuffer::kWrite);
    GledNS::WriteLensID(b, l);
    mSaturn->ShootMIRResult(b);
  }
  return l;
}

void ZDeque::PushBack(ZGlass* lens)
{
  GMutexHolder llck(mListMutex);
  new_element_check(lens);
  lens->IncRefCount(this);
  mElements.push_back(lens); ++mSize;
  on_insert(--end());
  StampListPushBack(lens);
}

ZGlass* ZDeque::PopBack()
{
  static const Exc_t _eh("ZDeque::PopBack ");

  if(mSize == 0)
    throw(_eh + "deque empty.");

  ZGlass* l;
  { GMutexHolder llck(mListMutex);
    l = mElements.back();
    on_remove(--end());
    mElements.pop_back(); --mSize;
    l->DecRefCount(this);
    StampListPopBack();
  }
  ZMIR* mir = get_MIR();
  if(mir && mir->HasResultReq()) {
    TBufferFile b(TBuffer::kWrite);
    GledNS::WriteLensID(b, l);
    mSaturn->ShootMIRResult(b);
  }
  return l;
}

void ZDeque::PushFront(ZGlass* lens)
{
  GMutexHolder llck(mListMutex);
  new_element_check(lens);
  lens->IncRefCount(this);
  mElements.push_front(lens); ++mSize;
  on_insert(begin());
  StampListPushFront(lens);
}

ZGlass* ZDeque::PopFront()
{
  static const Exc_t _eh("ZDeque::PopFront ");

  if(mSize == 0)
    throw(_eh + "deque empty.");

  ZGlass* l;
  { GMutexHolder llck(mListMutex);
    l = mElements.front();
    on_remove(begin());
    mElements.pop_front(); --mSize;
    l->DecRefCount(this);
    StampListPopFront();
  }
  ZMIR* mir = get_MIR();
  if(mir && mir->HasResultReq()) {
    TBufferFile b(TBuffer::kWrite);
    GledNS::WriteLensID(b, l);
    mSaturn->ShootMIRResult(b);
  }
  return l;
}

/**************************************************************************/
// ROOT
/**************************************************************************/

void ZDeque::Streamer(TBuffer &b)
{
  static const Exc_t _eh("ZDeque::Streamer ");
  UInt_t R__s, R__c;

  if(b.IsReading()) {

    Version_t R__v = b.ReadVersion(&R__s, &R__c); if(R__v) { }
    AList::Streamer(b);
    ID_t id;
    mElements.clear();
    for(Int_t i=0; i<mSize; ++i) {
      b >> id;
      mElements.push_back((ZGlass*)id);
    }
    b.CheckByteCount(R__s, R__c, ZDeque::IsA());

  } else {

    R__c = b.WriteVersion(ZDeque::IsA(), kTRUE);
    AList::Streamer(b);
    for(iterator i=begin(); i!=end(); ++i)
      b << i()->GetSaturnID();
    b.SetByteCount(R__c, kTRUE);

  }
}

