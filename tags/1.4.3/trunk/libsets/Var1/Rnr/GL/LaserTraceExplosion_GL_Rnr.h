// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_LaserTraceExplosion_GL_RNR_H
#define Var1_LaserTraceExplosion_GL_RNR_H

#include <Glasses/LaserTraceExplosion.h>
#include <Rnr/GL/Explosion_GL_Rnr.h>

class LaserTraceExplosion_GL_Rnr : public Explosion_GL_Rnr
{
private:
  void _init();

protected:
  LaserTraceExplosion*	mLaserTraceExplosion;

public:
  LaserTraceExplosion_GL_Rnr(LaserTraceExplosion* idol);
  virtual ~LaserTraceExplosion_GL_Rnr();

  //virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  //virtual void PostDraw(RnrDriver* rd);

  virtual void Render(RnrDriver* rd);

}; // endclass LaserTraceExplosion_GL_Rnr

#endif
