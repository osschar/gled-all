// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZHashList_H
#define Gled_ZHashList_H

#include <Glasses/ZList.h>

class ZHashList : public ZList {
  // 7777 RnrCtrl(1)
  MAC_RNR_FRIENDS(ZHashList);

private:
  void _init();

protected:
#ifndef __CINT__
  hash_map<ZGlass*, lpZGlass_i >	mItHash;
#endif

  virtual void clear_list();

  virtual void remove_references_to(ZGlass* lens);

  bool	bNerdyListOps;	// X{GS} 7 Bool()

public:
  ZHashList(const Text_t* n="ZHashList", const Text_t* t=0) : ZList(n,t)
  { _init(); }

  virtual void Add(ZGlass* g);			     // X{E} C{1}
  virtual void AddBefore(ZGlass* g, ZGlass* before); // X{E} C{2}
  virtual void AddFirst(ZGlass* g);		     // X{E} C{1}
  virtual void Remove(ZGlass* g);		     // X{E} C{1}
  virtual void RemoveLast(ZGlass* g);		     // X{E} C{1}

  virtual Bool_t Has(ZGlass* g);

  ZGlass* After(ZGlass* g);
  ZGlass* Before(ZGlass* g);

  virtual Int_t RebuildListRefs(An_ID_Demangler* idd);

#include "ZHashList.h7"
  ClassDef(ZHashList, 1)
}; // endclass ZHashList

GlassIODef(ZHashList);

#endif
