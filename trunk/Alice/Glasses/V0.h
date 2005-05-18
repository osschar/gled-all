// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_V0_H
#define Alice_V0_H

#include <Glasses/ZNode.h>
#include <Glasses/RecTrack.h>

#include <AliESDv0.h>

class V0 : public RecTrack {
  MAC_RNR_FRIENDS(V0);

 private:
  void _init();
  
 protected:
  RecTrack*    mPChild;
  RecTrack*    mNChild;

 public:
  V0(const Text_t* n="V0", const Text_t* t=0) :
    RecTrack(n,t) { _init(); }

  V0(AliESDv0* t, const Text_t* n="V0", const Text_t* t=0);

#include "V0.h7"
  ClassDef(V0, 1)
    }; // endclass V0

GlassIODef(V0);

#endif
