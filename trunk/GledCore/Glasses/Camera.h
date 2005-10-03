// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Camera_H
#define GledCore_Camera_H

#include <Glasses/ZNode.h>
#include <Stones/ZTrans.h>

class Camera : public ZNode {
  MAC_RNR_FRIENDS(Camera);
  friend class Pupil;

public:
  Camera(const Text_t* n="Camera", const Text_t* t=0) : ZNode(n, t) {}
  
  void Home(); // X{E} 7 MButt()

#include "Camera.h7"
  ClassDef(Camera, 1)
}; // endclass Camera


#endif
