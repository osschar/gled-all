// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZVector
//
//

#include "ZVector.h"
#include "ZVector.c7"

ClassImp(ZVector)

/**************************************************************************/

void ZVector::_init()
{
  mGrowFac  = 1;
  mGrowStep = 4;
  mReserved = 0;
  mNextIdx  = 0;
}

/**************************************************************************/


Int_t ZVector::remove_references_to(ZGlass* lens)
{
  Int_t n  = ZGlass::remove_references_to(lens);
  GMutexHolder llck(mListMutex);
  for(iterator i=begin(); i!=end(); ++i) {
    if(i() == lens) {
      on_remove(i.idx);
      StampListElementSet(0, i.idx);
      ++n;
    }
  }
  return n;
}

void ZVector::clear_list()
{
  mSize     = 0;
  mReserved = 0;
  mNextIdx  = 0;
  container foo;
  mElements.swap(foo);
}

/**************************************************************************/

Int_t ZVector::RebuildListRefs(An_ID_Demangler* idd)
{
  Int_t ret = 0;
  for(iterator i = begin(); i != end(); ++i) {
    ZGlass* lens = 0;
    if(i() != 0) {
      lens = idd->DemangleID(GledNS::CastLens2ID(i()));
      if(lens) {
        try {
          lens->IncRefCount(this);
        }
        catch(Exc_t& exc) {
          lens = 0;
          ++ret;
        }
      }
      *i = lens;
    }
  }
  on_rebuild();
  return ret;
}

void ZVector::ClearList()
{
  static const Exc_t _eh("ZVector::ClearList ");

  if(mSize == 0) return;

  container foo;
  ISdebug(0, _eh + GForm("locking list '%s'.", GetName()));
  mListMutex.Lock();
  mElements.swap(foo);
  clear_list();
  StampListClear();
  Stamp();
  mListMutex.Unlock();
  ISdebug(0, _eh + GForm("unlocked list '%s'.", GetName()));
  for(vpZGlass_i i=foo.begin(); i!=foo.end(); ++i) {
    if(*i) (*i)->DecRefCount(this);
  }
  ISdebug(0, _eh + GForm("finished for '%s'.", GetName()));
}

/**************************************************************************/
// AList methods, public part.
/**************************************************************************/

AList::stepper_base* ZVector::make_stepper_imp(bool return_zeros)
{
  return new stepper_imp<ZVector>(begin(), end(), return_zeros);
}

/**************************************************************************/
// Generick AList
/**************************************************************************/

void ZVector::Add(ZGlass* lens)
{
  // Adds lens at the position mNextIdx, possibly extending the vector.

  static const Exc_t _eh("ZVector::Add ");

  GMutexHolder llck(mListMutex);
  bool growp    = false;
  bool reservep = false;
  if(mNextIdx >= mSize) {
    if(mSize >= mReserved) {
      if(mGrowFac == 1 && mGrowStep == 0)
        throw(_eh + "container full, auto-grow disabled.");
      reservep = true;
    }
    growp = true;
  }
  new_element_check(lens);
  if(lens) lens->IncRefCount(this);
  if(growp) {
    if(reservep) {
      mReserved = TMath::Max((Int_t)(mGrowFac*mSize + mGrowStep), mReserved + 1);
      mElements.reserve(mReserved);
    }
    mElements.push_back(lens); ++mSize;
    on_insert(mNextIdx);
    StampListPushBack(lens, mNextIdx++);
  } else {
    mElements[mNextIdx] = lens;
    on_insert(mNextIdx);
    StampListElementSet(lens, mNextIdx++);
  }
  Stamp();
}

void ZVector::RemoveAll(ZGlass* lens)
{
  Int_t n=0;
  GMutexHolder llck(mListMutex);
  for(iterator i = begin(); i != end(); ++i) {
    if(i() == lens) {
      on_remove(i.idx);
      mElements[i.idx] = 0;
      StampListElementSet(0, i.idx);
      ++n;
    }
  }
  if(n) lens->DecRefCount(this, n);
}

/**************************************************************************/
// By-id operations
/**************************************************************************/

void ZVector::SetElementById(ZGlass* lens, Int_t index)
{
  static const Exc_t _eh("ZVector::SetElementById ");

  GMutexHolder llck(mListMutex);
  if(index < 0 || index >= mSize)
    throw(_eh + "index out of range.");
  new_element_check(lens);
  if(lens) lens->IncRefCount(this);
  ZGlass* old_lens = mElements[index];
  mElements[index] = lens;
  if(old_lens) old_lens->DecRefCount(this);
  on_change(index, old_lens);
  StampListElementSet(lens, index);
  if(index >= mNextIdx) {
    mNextIdx = index + 1;
    Stamp(FID());
  }
}

ZGlass* ZVector::GetElementById(Int_t index)
{
  static const Exc_t _eh("ZVector::GetElementById ");

  ZGlass* l;
  {
    GMutexHolder llck(mListMutex);
    if(index < 0 || index >= mSize)
      throw(_eh + "index out of range.");
    l = mElements[index];
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
// ZVector
/**************************************************************************/

void ZVector::Resize(Int_t size)
{
  static const Exc_t _eh("ZVector::Resize ");

  if(size < 0) size = 0;

  GMutexHolder llck(mListMutex);

  if(size == mSize)
    return;

  if(size == 0) {
    ClearList();
    return;
  }

  if(size > mSize) {
    mElements.resize(size);
    mSize = size;
    for(Int_t idx = mNextIdx; idx < mSize; ++idx) {
      on_insert(idx);
      StampListPushBack(0, idx);
    }
    Stamp();
    return;
  }

  if(size < mSize) {
    for(Int_t i=mSize-1; i>=size; --i) {
      ZGlass* l = mElements[i];
      on_remove(i);
      mElements[i] = 0;
      if(l) l->DecRefCount(this);
      StampListPopBack();
    }
    mElements.resize(size);
    mSize = size;
    if(mNextIdx > mSize)
      mNextIdx = mSize;
    Stamp();
    return;
  }

  assert(false);
}

void ZVector::Reserve(Int_t reserve)
{
  mElements.reserve(reserve);
  mReserved = mElements.capacity();
}

void ZVector::ClipReserve()
{
  container foo = mElements;
  mElements.swap(foo);
  mReserved = mElements.capacity();
}

void ZVector::SetNextIdx(Int_t nextidx)
{
  if(nextidx < 0) nextidx = 0;
  if(nextidx > mSize) nextidx = mSize;
  mNextIdx = nextidx;
  Stamp(FID());
}

Int_t ZVector::FindFirstEmptyId(Int_t index)
{
  // Returns the first empty id (with 0 lens pointer) at or after index.
  // Default value for index = 0, so it starts at the beginning.
  // Returns -1 if there are no empty ids.

  while (index < mSize)
  {
    if (mElements[index] == 0) return index;
    ++index;
  }
  return -1;
}

Int_t ZVector::CountEmptyIds()
{
  // Returns number of empty ids (with 0 lens pointer).

  Int_t count = 0;
  for (Int_t i=0; i<mSize; ++i)
  {
    if (mElements[i] == 0) ++count;
  }
  return count;
}

/**************************************************************************/

void ZVector::dumpVecStat()
{
  printf("%s size=%zu (mSize=%d), capacity=%zu (mReserved=%d)\n",
         GetName(), mElements.size(), mSize, mElements.capacity(), mReserved);
}

/**************************************************************************/
/**************************************************************************/
// Streamer
/**************************************************************************/

void ZVector::Streamer(TBuffer &b)
{
  UInt_t R__s, R__c;

  if(b.IsReading()) {

    Version_t R__v = b.ReadVersion(&R__s, &R__c); if(R__v) { }
    AList::Streamer(b);
    b >> mGrowFac >> mGrowStep >> mReserved >> mNextIdx;
    TString str;
    ID_t    id;
    container foo;
    mElements.swap(foo);
    mElements.reserve(mReserved);
    mElements.resize(mSize);
    for(Int_t i=0; i<mSize; ++i) {
      b >> id >> str;
      mElements[i] = GledNS::CastID2Lens(id);
    }
    b.CheckByteCount(R__s, R__c, ZVector::IsA());

  } else {

    R__c = b.WriteVersion(ZVector::IsA(), kTRUE);
    AList::Streamer(b);
    b << mGrowFac << mGrowStep << mReserved << mNextIdx;
    for(iterator i=begin(); i!=end(); ++i)
      b << (i() ? i()->GetSaturnID() : (ID_t)0);
    b.SetByteCount(R__c, kTRUE);

  }
}

/**************************************************************************/
