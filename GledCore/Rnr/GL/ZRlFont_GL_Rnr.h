// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZRlFont_GL_RNR_H
#define GledCore_ZRlFont_GL_RNR_H

#include <Glasses/ZRlFont.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>
#include <GledView/GLTextNS.h>

class ZRlFont_GL_Rnr : public ZGlass_GL_Rnr {
private:
  void _init();

protected:
  ZRlFont*		mZRlFont;
  GLTextNS::TexFont*	mFont;
  GLTextNS::TexFont*	mExFont;

public:
  ZRlFont_GL_Rnr(ZRlFont* idol) :
    ZGlass_GL_Rnr(idol), mZRlFont(idol) { _init(); }

  virtual void AbsorbRay(Ray& ray);

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

  bool LoadFont();

  // Interface for PupilInfo_GL_Rnr
  void MakeDefault(RnrDriver* rd);

}; // endclass ZRlFont_GL_Rnr

#endif
