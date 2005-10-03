// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Audio1_AlListener_H
#define Audio1_AlListener_H

#include <Glasses/ZNode.h>

class AlListener : public ZNode {
  MAC_RNR_FRIENDS(AlListener);

public:
  enum LocationType_e { LT_Camera, LT_Absolute };

private:
  void _init();

protected:
  LocationType_e mLocationType; // X{GS} 7 PhonyEnum()

  Float_t        mGain; // X{GS} Ray{Source} 7 Value(-range=>[0,100,1,1000])

public:
  AlListener(const Text_t* n="AlListener", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  void EmitSourceRay();

#include "AlListener.h7"
  ClassDef(AlListener, 1)
}; // endclass AlListener


#endif
