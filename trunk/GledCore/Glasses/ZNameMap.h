// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZNameMap_H
#define GledCore_ZNameMap_H

#include <Glasses/ZList.h>

class ZNameMap : public ZList, public ZGlass::YNameChangeCB
{
  MAC_RNR_FRIENDS(ZNameMap);

private:
  void _init();

protected:
#ifndef __CINT__
  map<string, lpZGlass_i>		mItMap;
#endif

  virtual void clear_list();

  virtual Int_t remove_references_to(ZGlass* lens);

  lpZGlass_i insert(ZGlass* g, const string& name);

public:
  ZNameMap(const Text_t* n="ZNameMap", const Text_t* t=0) : ZList(n,t)
  { _init(); }

  virtual ZGlass* GetElementByName(const string& name);

  virtual void Insert(ZGlass* g);		     // X{E} C{1}

  virtual void Add(ZGlass* g);			     // X{E} C{1}
  virtual void AddBefore(ZGlass* g, ZGlass* before); // X{E} C{2}
  virtual void AddFirst(ZGlass* g);		     // X{E} C{1}
  virtual void Remove(ZGlass* g);		     // X{E} C{1}
  virtual void RemoveLast(ZGlass* g);		     // X{E} C{1}

  virtual void SortByName() {}                       // X{E}

  void RecreateMap();

  virtual Int_t RebuildListRefs(An_ID_Demangler* idd);

  virtual void y_name_change_cb(ZGlass* g, const string& new_name);

#include "ZNameMap.h7"
  ClassDef(ZNameMap, 1)
}; // endclass ZNameMap

GlassIODef(ZNameMap);

#endif
