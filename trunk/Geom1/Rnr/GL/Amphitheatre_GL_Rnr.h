// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_Amphitheatre_GL_RNR_H
#define Geom1_Amphitheatre_GL_RNR_H

#include <Glasses/Amphitheatre.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

#include <GL/glu.h>

class Amphitheatre_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  Amphitheatre*	 mAmphitheatre;
  GLUquadricObj* mQuadric;

public:
  Amphitheatre_GL_Rnr(Amphitheatre* idol) :
    ZNode_GL_Rnr(idol), mAmphitheatre(idol) { _init(); }

  //virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  //virtual void PostDraw(RnrDriver* rd);

}; // endclass Amphitheatre_GL_Rnr

#endif
