// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_GLTesterOne_GL_RNR_H
#define Geom1_GLTesterOne_GL_RNR_H

#include <Glasses/GLTesterOne.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class GLTesterOne_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  GLTesterOne*	mGLTesterOne;

public:
  GLTesterOne_GL_Rnr(GLTesterOne* idol) :
    ZNode_GL_Rnr(idol), mGLTesterOne(idol)
  { _init(); }

  virtual void Draw(RnrDriver* rd);

  virtual void Render(RnrDriver* rd);

}; // endclass GLTesterOne_GL_Rnr

#endif
