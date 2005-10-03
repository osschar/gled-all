// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZVector_H
#define GledCore_ZVector_H

#include <Glasses/AList.h>

class ZVector : public AList
{
  MAC_RNR_FRIENDS(ZVector);

private:
  void _init();

protected:
  vpZGlass_t        mVector;

  virtual Int_t remove_references_to(ZGlass* lens) {return 0;}
  virtual void clear_list() {}

public:
  ZVector(const Text_t* n="ZVector", const Text_t* t=0) :
    AList(n,t) { _init(); }

  virtual Int_t RebuildListRefs(An_ID_Demangler* idd) {return 0;}
  virtual void ClearList() {}

  virtual void Add      (ZGlass* lens) {}
  virtual void RemoveAll(ZGlass* lens) {}

  virtual bool elrep_has_id()         { return true; }

#include "ZVector.h7"
  ClassDef(ZVector, 1)
}; // endclass ZVector


#endif
