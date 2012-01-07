// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_SRefCounted_H
#define GledCore_SRefCounted_H

#include <Rtypes.h>

class SRefCounted
{
protected:
  Int_t   mRefCount; //!

public:
  SRefCounted();
  virtual ~SRefCounted();

  virtual void SetRefCount(Int_t rc);

  virtual void IncRefCount();
  virtual void DecRefCount();
  virtual void OnZeroRefCount();

  ClassDef(SRefCounted, 0);
}; // endclass SRefCounted


class SRefCountedNV
{
protected:
  Int_t   mRefCount; //!

public:
  SRefCountedNV() : mRefCount(0) {}

  void SetRefCount(Int_t rc) { mRefCount = rc; }

  void IncRefCount() { ++mRefCount; }
  void DecRefCount() { --mRefCount; if (mRefCount <= 0) delete this; }

  ClassDefNV(SRefCountedNV, 0);
}; // endclass SRefCountedNV

#endif
