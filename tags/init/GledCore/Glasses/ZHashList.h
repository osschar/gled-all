// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZHashList_H
#define Gled_ZHashList_H

#include <Glasses/ZList.h>

class ZHashList : public ZList {
  // 7777 RnrCtrl("false, 7, RnrBits(2,4,6,0, 0,0,0,5)")
  MAC_RNR_FRIENDS(ZHashList);
private:
  void _init();
protected:
#ifndef __CINT__
  hash_map<ZGlass*, lpZGlass_i >	mItHash;
#endif

  bool	bNerdyListOps;	// X{GS} 7 Bool()

public:
  ZHashList(const Text_t* n="ZHashList", const Text_t* t=0) : ZList(n,t)
  { _init(); }

  virtual void Add(ZGlass* g);			     // X{E} C{1}
  virtual void AddBefore(ZGlass* g, ZGlass* before); // X{E} C{2}
  virtual void AddFirst(ZGlass* g);		     // X{E} C{1}
  virtual void Remove(ZGlass* g);		     // X{E} C{1}
  virtual void RemoveLast(ZGlass* g);		     // X{E} C{1}

  virtual bool	Has(ZGlass* g);

  virtual Int_t RebuildList(ZComet* c);

#include "ZHashList.h7"
  ClassDef(ZHashList, 1)
}; // endclass ZHashList

GlassIODef(ZHashList);

#endif
