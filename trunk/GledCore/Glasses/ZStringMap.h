// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZStringMap_H
#define GledCore_ZStringMap_H

#include <Glasses/AList.h>

class ZStringMap : public AList
{
  MAC_RNR_FRIENDS(ZStringMap);

private:
  void _init();

protected:
  virtual Int_t remove_references_to(ZGlass* lens) {return 0;}
  virtual void clear_list() {}

public:
  ZStringMap(const Text_t* n="ZStringMap", const Text_t* t=0) :
    AList(n,t) { _init(); }

  virtual Int_t RebuildListRefs(An_ID_Demangler* idd) {return 0;}
  virtual void ClearList() {}

  virtual void Add      (ZGlass* lens) {}
  virtual void RemoveAll(ZGlass* lens) {}

#include "ZStringMap.h7"
  ClassDef(ZStringMap, 1)
}; // endclass ZStringMap


#endif
