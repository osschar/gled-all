// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
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
  // !!!! Set all links to 0 !!!!
}

/**************************************************************************/

void ZDeque::CopyList(lpZGlass_t& dest)
{
  mListMutex.Lock();
  copy(mLenses.begin(), mLenses.end(), back_inserter(dest));
  mListMutex.Unlock();
}

void ZDeque::CopyListElRefs(lElRep_t& dest)
{

}

/**************************************************************************/

Int_t ZDeque::RebuildListRefs(An_ID_Demangler* idd)
{
  Int_t ret = 0;
  mLenses.clear(); mSize = 0;
  for(lID_i i=mIDs.begin(); i!=mIDs.end(); ++i) {
    ZGlass* lens = idd->DemangleID(*i);
    if(lens) {
      try {
	lens->IncRefCount(this);
	mLenses.push_back(lens); ++mSize;
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

void    ZDeque::PushBack(ZGlass* lens) {}
ZGlass* ZDeque::PopBack() {}
void    ZDeque::PushFront(ZGlass* lens) {}
ZGlass* ZDeque::PopFront() {}

/**************************************************************************/

void ZDeque::Streamer(TBuffer &b)
{
  static const string _eh("ZDeque::Streamer ");
  UInt_t R__s, R__c;

  if(b.IsReading()) {

    Version_t v = b.ReadVersion(&R__s, &R__c);
    AList::Streamer(b);
    mIDs.clear();
    ID_t id;
    for(Int_t i=0; i<mSize; i++) { b >> id; mIDs.push_back(id); }
    b.CheckByteCount(R__s, R__c, ZDeque::IsA());

  } else {

    R__c = b.WriteVersion(ZDeque::IsA(), kTRUE);
    AList::Streamer(b);
    for(lpZGlass_i i=mLenses.begin(); i!=mLenses.end(); i++)
      b << (*i)->GetSaturnID();
    b.SetByteCount(R__c, kTRUE);

  }
}

