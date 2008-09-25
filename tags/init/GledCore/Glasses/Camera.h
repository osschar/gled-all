// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Camera_H
#define Gled_Camera_H

#include <Glasses/ZNode.h>
#include <Stones/ZTrans.h>

class Camera : public ZNode {
  // 7777 RnrCtrl("true, 7, RnrBits(2,4,6,0, 0,0,0,5)")
  MAC_RNR_FRIENDS(Camera);
  friend class Pupil;
private:
public:
  // virtual void Draw(); Cameras are dummy objects ... if any sons ... draws them
  // perhaps will need sth special for tubular links
  Camera(const Text_t* n="Camera", const Text_t* t=0) : ZNode(n, t) {}
  
  void Home();

#include "Camera.h7"
  ClassDef(Camera, 1)
}; // endclass Camera

GlassIODef(Camera);

#endif
