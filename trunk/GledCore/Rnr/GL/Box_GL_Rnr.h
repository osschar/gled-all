// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Box_GL_RNR_H
#define GledCore_Box_GL_RNR_H

#include <Glasses/Box.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class Box_GL_Rnr : public ZNode_GL_Rnr {
private:

protected:
  Box*	mBox;

public:
  Box_GL_Rnr(Box* idol) : ZNode_GL_Rnr(idol), mBox(idol) {}

  virtual void Draw(RnrDriver* rd);

}; // endclass Box_GL_Rnr

#endif
