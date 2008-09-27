// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZGod_H
#define Gled_ZGod_H

#include <Glasses/ZHashList.h>

class ZGod : public ZHashList {
  MAC_RNR_FRIENDS(ZGod);
private:
protected:
public:
  ZGod(const Text_t* n="ZGod", const Text_t* t=0) : ZHashList(n,t) {}


#include "ZGod.h7"
  ClassDef(ZGod, 1)
}; // endclass ZGod


#endif
