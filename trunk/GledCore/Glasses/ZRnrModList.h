// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZRnrModList_H
#define GledCore_ZRnrModList_H

#include <Glasses/ZHashList.h>

class ZRnrModList : public ZHashList {
  MAC_RNR_FRIENDS(ZRnrModList);

private:
  void _init();

protected:

public:
  ZRnrModList(const Text_t* n="ZRnrModList", const Text_t* t=0) :
    ZHashList(n,t) { _init(); }


#include "ZRnrModList.h7"
  ClassDef(ZRnrModList, 1) // List of lenses that modify renderer-state
}; // endclass ZRnrModList

GlassIODef(ZRnrModList);

#endif
