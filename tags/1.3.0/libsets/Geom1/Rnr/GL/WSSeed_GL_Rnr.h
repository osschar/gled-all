// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_WSSeed_GL_RNR_H
#define Geom1_WSSeed_GL_RNR_H

#include <Glasses/WSSeed.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class WSSeed_GL_Rnr : public ZNode_GL_Rnr
{
protected:
  WSSeed*	mWSSeed;

  void vert(WSPoint* f, Float_t t);

public:
  WSSeed_GL_Rnr(WSSeed* idol) : ZNode_GL_Rnr(idol), mWSSeed(idol) {}

  virtual void Draw(RnrDriver* rd);

  virtual void Render(RnrDriver* rd);
  virtual void Triangulate(RnrDriver* rd);

}; // endclass WSSeed_GL_Rnr

#endif
