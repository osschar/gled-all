// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_CosmicBall_GL_RNR_H
#define Var1_CosmicBall_GL_RNR_H

#include <Glasses/CosmicBall.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

#include <GL/gl.h>
#include <GL/glu.h>

class CosmicBall_GL_Rnr : public ZNode_GL_Rnr
{
private:
  void _init();

protected:
  CosmicBall*	mCosmicBall;

  static GLUquadricObj* sQuadric;

public:
  CosmicBall_GL_Rnr(CosmicBall* idol) :
    ZNode_GL_Rnr(idol), mCosmicBall(idol)
  { _init(); }

  //virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass CosmicBall_GL_Rnr

#endif
