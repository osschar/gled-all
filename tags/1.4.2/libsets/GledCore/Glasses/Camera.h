// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Camera_H
#define GledCore_Camera_H

#include <Glasses/ZNode.h>
#include <Stones/ZTrans.h>

class Camera : public ZNode
{
  MAC_RNR_FRIENDS(Camera);
  friend class Pupil;

protected:
  ZTrans  mHomeTrans; // X{R}

public:
  Camera(const Text_t* n="Camera", const Text_t* t=0) : ZNode(n, t) {}

  void Home();     // X{E} 7 MButt(-join=>1)
  void Identity(); // X{E} 7 MButt()

  void SetHomeTrans();   // X{E} 7 MButt(-join=>1)
  void ResetHomeTrans(); // X{E} 7 MButt()

#include "Camera.h7"
  ClassDef(Camera, 1);
}; // endclass Camera


#endif
