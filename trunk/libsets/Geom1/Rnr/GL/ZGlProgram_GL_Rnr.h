// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_ZGlProgram_GL_RNR_H
#define Geom1_ZGlProgram_GL_RNR_H

#include <Glasses/ZGlProgram.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>

class ZGlProgram_GL_Rnr : public ZGlass_GL_Rnr
{
private:
  void _init();

protected:
  ZGlProgram*	mZGlProgram;

  Bool_t        bRelink;
  Bool_t        bRecompile;
  Bool_t        bBuildFailed;

  GLuint        mProgID;

public:
  ZGlProgram_GL_Rnr(ZGlProgram* idol);
  virtual ~ZGlProgram_GL_Rnr();

  virtual void AbsorbRay(Ray& ray);

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass ZGlProgram_GL_Rnr

#endif
