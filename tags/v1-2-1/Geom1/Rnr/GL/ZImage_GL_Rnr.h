// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_ZImage_GL_RNR_H
#define Geom1_ZImage_GL_RNR_H

#include <Glasses/ZImage.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>

#include <GL/gl.h>

class ZImage_GL_Rnr : public ZGlass_GL_Rnr {
private:
  void _init();

protected:
  ZImage*	mImage;
  TimeStamp_t	mStampTexture;
  GLuint	mTexture;

  void init_texture();

public:
  ZImage_GL_Rnr(ZImage* idol) : ZGlass_GL_Rnr(idol), mImage(idol) { _init(); }
  virtual ~ZImage_GL_Rnr();

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass ZImage_GL_Rnr

#endif
