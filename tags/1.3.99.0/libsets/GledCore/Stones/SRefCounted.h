// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_SRefCounted_H
#define GledCore_SRefCounted_H

#include <Rtypes.h>
#include "Gled/GMutex.h"

class SRefCounted
{
protected:
  GMutex  mRCMutex;  //!
  Int_t   mRefCount; //!

public:
  SRefCounted();
  virtual ~SRefCounted();

  virtual void  SetRefCount(Int_t rc);

  virtual Int_t IncRefCount(Int_t rc=1);
  virtual Int_t DecRefCount(Int_t rc=1);

  virtual void  OnZeroRefCount();

  ClassDef(SRefCounted, 0);
}; // endclass SRefCounted


class SRefCountedNV
{
protected:
  GMutex  mRCMutex;  //!
  Int_t   mRefCount; //!

public:
  SRefCountedNV() : mRefCount(0) {}

  void  SetRefCount(Int_t rc)   { GMutexHolder _lck(mRCMutex); mRefCount = rc; }
  Int_t IncRefCount(Int_t rc=1) { GMutexHolder _lck(mRCMutex); mRefCount += rc; return mRefCount; }

  // DecRefCount *must* be implemented in derived class. See define below.
  // Int_t DecRefCount(Int_t rc=1);

  ClassDefNV(SRefCountedNV, 0);
}; // endclass SRefCountedNV

#define SRefCountedNV_DecRefCount_macro \
  Int_t DecRefCount(Int_t rc=1) \
  {				\
    mRCMutex.Lock();		\
    mRefCount -= rc;		\
    Int_t ret = mRefCount;	\
    if (mRefCount <= 0)		\
      delete this;		\
    else			\
      mRCMutex.Unlock();	\
    return ret;			\
  }

#endif
