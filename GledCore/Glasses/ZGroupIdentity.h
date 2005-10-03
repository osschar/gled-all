// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGroupIdentity_H
#define GledCore_ZGroupIdentity_H

#include <Glasses/ZIdentity.h>

class ZGroupIdentity : public ZIdentity {
  MAC_RNR_FRIENDS(ZGroupIdentity);

  friend class ZSunQueen;

private:
  void _init();

protected:

public:
  ZGroupIdentity(const Text_t* n="ZGroupIdentity", const Text_t* t=0) :
    ZIdentity(n,t) { _init(); }

#include "ZGroupIdentity.h7"
  ClassDef(ZGroupIdentity, 1) // Representation of a group identity
}; // endclass ZGroupIdentity


#endif
