// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Numerica_Moonraker_GL_RNR_H
#define Numerica_Moonraker_GL_RNR_H

#include <Glasses/Moonraker.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

#include <FL/gl.h>
#include <GL/glu.h>

class Moonraker_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  Moonraker*	 mMoonraker;

  GLUquadricObj* mQuadric;

public:
  Moonraker_GL_Rnr(Moonraker* idol) : ZNode_GL_Rnr(idol), mMoonraker(idol)
  { _init(); }
  virtual ~Moonraker_GL_Rnr();

  virtual void Draw(RnrDriver* rd);

}; // endclass Moonraker_GL_Rnr

#endif
