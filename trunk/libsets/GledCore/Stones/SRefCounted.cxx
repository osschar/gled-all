// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SRefCounted.h"

//__________________________________________________________________________
//
// A reference-counted stone.
// Note that the ref-count is not serialized -- so rebuild it in
// post-streaming function when necessary.
// When zero ref count is reached in DecRefCount(),
//   virtual void OnZeroRefCount();
// is called. Default action is to destroy the object. Note, mutex is NOT
// unlocked in this case ... so if you keep the object alive in OnZeroRefCnt()
// also unlock the mutex.

ClassImp(SRefCounted);

SRefCounted::SRefCounted() : mRefCount(0)
{}

SRefCounted::~SRefCounted()
{}

void SRefCounted::SetRefCount(Int_t rc)
{
  GMutexHolder _lck(mRCMutex);
  mRefCount = rc;
}

Int_t SRefCounted::IncRefCount(Int_t rc)
{
  GMutexHolder _lck(mRCMutex);
  mRefCount += rc;
  return mRefCount;
}

Int_t SRefCounted::DecRefCount(Int_t rc)
{
  mRCMutex.Lock();
  mRefCount -= rc;
  Int_t ret = mRefCount;
  if (mRefCount <= 0)
    OnZeroRefCount();
  else
    mRCMutex.Unlock();
  return ret;
}

void SRefCounted::OnZeroRefCount()
{
  delete this;
}


//__________________________________________________________________________
//
// A reference-counted stone without virtual functions, all functions inline.
// Note that the ref-count is not serialized -- so rebuild it in
// post-streaming function when necessary.
// As there are no virtual functions
//   Int_t DecRefCount(Int_t rc=1);
// *must* be implmented in the sub-class. Macro
//   SRefCountedNV_DecRefCount_macro
// is defined for this purpose in the header file. This implementation
// destructs the object when zero ref count is reached.

ClassImp(SRefCountedNV);
