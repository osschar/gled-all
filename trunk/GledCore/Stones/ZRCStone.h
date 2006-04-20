// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZRCStone_H
#define GledCore_ZRCStone_H

#include <TObject.h>

class ZRCStone : public TObject 
{
protected:
  Int_t   mRefCount; 

public:
  ZRCStone() : TObject(), mRefCount(0) {}

  void IncRefCount() { ++mRefCount; }
  void DecRefCount() { --mRefCount; if(mRefCount <= 0) delete this; }

#include "ZRCStone.h7"
  ClassDef(ZRCStone, 1)
}; // endclass ZRCStone

#endif
