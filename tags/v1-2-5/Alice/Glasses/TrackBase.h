// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_TrackBase_H
#define Alice_TrackBase_H

#include <Glasses/ZNode.h>

class TrackBase : public ZNode {
  MAC_RNR_FRIENDS(TrackBase);

private:
  void _init();

protected:

public:
  TString                  mV;             // X{GS}  7 TextOut()
  TString                  mP;             // X{GS}  7 TextOut()

  TrackBase(const Text_t* n="TrackBase", const Text_t* t=0) :
    ZNode(n,t) { _init(); }


#include "TrackBase.h7"
  ClassDef(TrackBase, 1)
}; // endclass TrackBase

GlassIODef(TrackBase);

#endif
