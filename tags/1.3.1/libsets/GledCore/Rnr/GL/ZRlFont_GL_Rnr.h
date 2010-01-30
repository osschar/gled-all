// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZRlFont_GL_RNR_H
#define GledCore_ZRlFont_GL_RNR_H

#include <Glasses/ZRlFont.h>
#include <Rnr/GL/ZRnrModBase_GL_Rnr.h>
#include <Rnr/GL/GLTextNS.h>

class ZRlFont_GL_Rnr : public ZRnrModBase_GL_Rnr
{
private:
  void _init();

protected:
  ZRlFont*		mZRlFont;
  GLTextNS::TexFont*	mFont;    // X{g}

public:
  ZRlFont_GL_Rnr(ZRlFont* idol) :
    ZRnrModBase_GL_Rnr(idol), mZRlFont(idol) { _init(); }

  virtual void AbsorbRay(Ray& ray);

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

  bool LoadFont();

#include "ZRlFont_GL_Rnr.h7"
}; // endclass ZRlFont_GL_Rnr

#endif
