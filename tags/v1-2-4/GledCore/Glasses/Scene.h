// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Scene_H
#define GledCore_Scene_H

#include <Glasses/ZNode.h>
#include <Glasses/GlobalLamps.h>

class Scene : public ZNode {
  // 7777 RnrCtrl(1)
  MAC_RNR_FRIENDS(Scene);

private:
  void _init();

protected:
  GlobalLamps*		mGlobLamps; // X{GS} L{l} RnrBits{3,0,7,0}

public:
  Scene(const Text_t* n="Scene", const Text_t* t=0) : ZNode(n,t) { _init(); }

  virtual void AdEnlightenment();

#include "Scene.h7"
  ClassDef(Scene, 1)
}; // endclass Scene

GlassIODef(Scene);

#endif
