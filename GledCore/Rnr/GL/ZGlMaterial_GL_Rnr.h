// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlMaterial_GL_RNR_H
#define GledCore_ZGlMaterial_GL_RNR_H

#include <Glasses/ZGlMaterial.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>

class ZGlMaterial_GL_Rnr : public ZGlass_GL_Rnr {
private:
  void _init();

protected:
  ZGlMaterial*	mZGlMaterial;

public:
  ZGlMaterial_GL_Rnr(ZGlMaterial* idol) : ZGlass_GL_Rnr(idol), mZGlMaterial(idol) {}

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

  void SetupGL();

}; // endclass ZGlMaterial_GL_Rnr

#endif
