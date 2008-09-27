// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Cylinder_GL_RNR_H
#define GledCore_Cylinder_GL_RNR_H

#include <Glasses/Cylinder.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

#include <GL/gl.h>
#include <GL/glu.h>

class Cylinder_GL_Rnr : public ZNode_GL_Rnr {
 private:
  void _init();

 protected:
  Cylinder*      mCylinder;
  GLUquadricObj* mQuadric;

 public:
  Cylinder_GL_Rnr(Cylinder* idol) :
    ZNode_GL_Rnr(idol), mCylinder(idol) { _init(); }
  virtual ~Cylinder_GL_Rnr();

  virtual void Render(RnrDriver* rd);
};

#endif
