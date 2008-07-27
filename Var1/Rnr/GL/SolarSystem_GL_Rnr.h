// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_SolarSystem_GL_RNR_H
#define Var1_SolarSystem_GL_RNR_H

#include <Glasses/SolarSystem.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class SolarSystem_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  SolarSystem*  mSolarSystem;

public:
  SolarSystem_GL_Rnr(SolarSystem* idol) :
    ZNode_GL_Rnr(idol), mSolarSystem(idol)
  { _init(); }

  //virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  //virtual void PostDraw(RnrDriver* rd);

}; // endclass SolarSystem_GL_Rnr

#endif
