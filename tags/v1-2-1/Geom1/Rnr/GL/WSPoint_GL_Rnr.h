// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_WSPoint_GL_RNR_H
#define Geom1_WSPoint_GL_RNR_H

#include <Glasses/WSPoint.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class WSPoint_GL_Rnr : public ZNode_GL_Rnr {
private:

protected:
  WSPoint*	mWSPoint;

public:
  WSPoint_GL_Rnr(WSPoint* idol) : ZNode_GL_Rnr(idol), mWSPoint(idol) {}

  virtual void Draw(RnrDriver* rd);

}; // endclass WSPoint_GL_Rnr

#endif
