// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZHashList
//
//

#include "ZHashList.h"
#include "ZHashList.c7"

ClassImp(ZHashList)

/**************************************************************************/

void ZHashList::_init()
{
  bNerdyListOps = true;
}

/**************************************************************************/

void ZHashList::new_element_check(ZGlass* lens)
{
  static const Exc_t _eh("ZHashList::new_element_check ");

  hpLens2Iter_i i = mItHash.find(lens);
  if(i != mItHash.end()) {
    throw(_eh + "lens " + lens->Identify() + " already in the list.");
  }
  PARENT_GLASS::new_element_check(lens);
}

void ZHashList::clear_list()
{
  PARENT_GLASS::clear_list();
  mItHash.clear();
}

/**************************************************************************/

Int_t ZHashList::remove_references_to(ZGlass* lens)
{
  Int_t n = ZGlass::remove_references_to(lens);

  GMutexHolder llck(mListMutex);
  hpLens2Iter_i i = mItHash.find(lens);
  if(i != mItHash.end()) {
    ZList::iterator l = i->second;
    mItHash.erase(i);
    mElements.erase(l); --mSize;
    StampListRemove(lens);
    ++n;
  }

  return n;
}

/**************************************************************************/

void ZHashList::on_insert(ZList::iterator it)
{
  mItHash[it.lens()] = it;
}

void ZHashList::on_remove(ZList::iterator it)
{
  hpLens2Iter_i i = mItHash.find(it.lens());
  assert(i != mItHash.end());
  mItHash.erase(i);
}

void ZHashList::on_rebuild()
{
  mItHash.clear();
  for(ZList::iterator i=begin(); i!=end(); ++i)
    mItHash[i.lens()] = i;
}

/**************************************************************************/

Bool_t ZHashList::Has(ZGlass* lens)
{
  GMutexHolder llck(mListMutex);
  hpLens2Iter_i i = mItHash.find(lens);
  return (i != mItHash.end());
}

/**************************************************************************/

Int_t ZHashList::RemoveAll(ZGlass* lens)
{
  GMutexHolder llck(mListMutex);
  hpLens2Iter_i i = mItHash.find(lens);
  if(i != mItHash.end()) {
    ZList::iterator l = i->second;
    mItHash.erase(i);
    mElements.erase(l); --mSize;
    lens->DecRefCount(this);
    StampListRemove(lens);
    return 1;
  } else {
    return 0;
  }
}

/**************************************************************************/

void ZHashList::Insert(ZGlass* lens, ZGlass* before)
{
  static const Exc_t _eh("ZHashList::Insert ");

  GMutexHolder llck(mListMutex);
  new_element_check(lens);
  hpLens2Iter_i i = mItHash.find(before);
  if(i == mItHash.end())
    throw(_eh + "before-lens " + before->Identify() + " not found in the list.");
  lens->IncRefCount(this);
  ZList::iterator l = i->second;
  mElements.insert(l, element(lens, mNextId++)); ++mSize;
  mItHash[lens] = --l;
  StampListInsert(lens, mNextId-1, before);
}

void ZHashList::Remove(ZGlass* lens)
{
  static const Exc_t _eh("ZHashList::Remove ");

  GMutexHolder llck(mListMutex);
  hpLens2Iter_i i = mItHash.find(lens);
  if(i == mItHash.end())
    throw(_eh + "lens " + lens->Identify() + " not found in the list.");
  ZList::iterator l = i->second;
  mItHash.erase(i);
  mElements.erase(l); --mSize;
  lens->DecRefCount(this);
  StampListRemove(lens);
}

/**************************************************************************/
/**************************************************************************/

ZGlass* ZHashList::ElementAfter(ZGlass* lens)
{
  ZGlass* ret = 0;
  { GMutexHolder llck(mListMutex);
    hpLens2Iter_i i = mItHash.find(lens);
    if(i != mItHash.end()) {
      ZList::iterator j(i->second);
      if(++j != end())
	ret = j.lens();
    }
  }
  ZMIR* mir = get_MIR();
  if(mir && mir->HasResultReq()) {
    TBufferFile b(TBuffer::kWrite);
    GledNS::WriteLensID(b, ret);
    mSaturn->ShootMIRResult(b);
  }
  return ret;
}

ZGlass* ZHashList::ElementBefore(ZGlass* lens)
{
  ZGlass* ret = 0;
  { GMutexHolder llck(mListMutex);
    hpLens2Iter_i i = mItHash.find(lens);
    if(i != mItHash.end()) {
      ZList::iterator j = i->second;
      if(j != begin())
	ret = (--j).lens();
    }
  }
  ZMIR* mir = get_MIR();
  if(mir && mir->HasResultReq()) {
    TBufferFile b(TBuffer::kWrite);
    GledNS::WriteLensID(b, ret);
    mSaturn->ShootMIRResult(b);
  }
  return ret;
}

/**************************************************************************/
