// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Explosion_GL_RNR_H
#define Var1_Explosion_GL_RNR_H

#include <Glasses/Explosion.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>

class Explosion_GL_Rnr : public ZGlass_GL_Rnr
{
private:
  void _init();

protected:
  Explosion*	mExplosion;

public:
  Explosion_GL_Rnr(Explosion* idol);
  virtual ~Explosion_GL_Rnr();

  // virtual void PreDraw(RnrDriver* rd);
  // virtual void Draw(RnrDriver* rd);
  // virtual void PostDraw(RnrDriver* rd);

  // virtual void Render(RnrDriver* rd);

}; // endclass Explosion_GL_Rnr

#endif
