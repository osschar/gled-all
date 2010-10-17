// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_EyeInfoVector_H
#define GledCore_EyeInfoVector_H

#include <Gled/GledTypes.h>
#include <Gled/GSpinLock.h>

class EyeInfo;

#include <vector>

class EyeInfoVector : public vector<EyeInfo*>
{
protected:
  GSpinLock  mLock;
  Int_t      mRefCnt;

public:
  EyeInfoVector(Int_t s=0) : vector<EyeInfo*>(s), mRefCnt(0) {}
  ~EyeInfoVector() {}

  void IncRefCnt() { ++mRefCnt; }
  void DecRefCnt() { mLock.Lock(); if (--mRefCnt == 0) delete this; else mLock.Unlock(); }

  static EyeInfoVector* CloneAndAdd(EyeInfoVector* eiv, EyeInfo* ei);
  static EyeInfoVector* CloneAndRemove(EyeInfoVector* eiv, EyeInfo* ei);

#include "EyeInfoVector.h7"
  ClassDef(EyeInfoVector, 0);
}; // endclass EyeInfoVector

#endif
