// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GlobalLamps_H
#define GledCore_GlobalLamps_H

#include <Glasses/ZHashList.h>
#include <Glasses/Lamp.h>

class GlobalLamps : public ZHashList
{
  MAC_RNR_FRIENDS(GlobalLamps);

private:
  void _init();

protected:

public:
  GlobalLamps(const Text_t* n="GlobalLamps", const Text_t* t=0) : ZHashList(n,t) { _init(); }

#include "GlobalLamps.h7"
  ClassDef(GlobalLamps, 1)
}; // endclass GlobalLamps


#endif
