// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Audio1_AlListener_H
#define Audio1_AlListener_H

#include <Glasses/ZNode.h>

class AlListener : public ZNode {
  MAC_RNR_FRIENDS(AlListener);

private:
  void _init();

protected:

public:
  AlListener(const Text_t* n="AlListener", const Text_t* t=0) :
    ZNode(n,t) { _init(); }


#include "AlListener.h7"
  ClassDef(AlListener, 1)
}; // endclass AlListener

GlassIODef(AlListener);

#endif
