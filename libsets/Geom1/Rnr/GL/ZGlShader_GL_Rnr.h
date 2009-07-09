// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_ZGlShader_GL_RNR_H
#define Geom1_ZGlShader_GL_RNR_H

#include <Glasses/ZGlShader.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>

class ZGlShader_GL_Rnr : public ZGlass_GL_Rnr
{
private:
  void _init();

protected:
  ZGlShader*	mZGlShader;

  Bool_t        bRecompile;
  GLuint	mShaderID;

public:
  ZGlShader_GL_Rnr(ZGlShader* idol);
  virtual ~ZGlShader_GL_Rnr();

  virtual void AbsorbRay(Ray& ray);

  virtual GLuint AssertShader();
  virtual GLuint Compile();
}; // endclass ZGlShader_GL_Rnr

#endif
