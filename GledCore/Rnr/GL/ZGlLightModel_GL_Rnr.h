// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlLightModel_GL_RNR_H
#define GledCore_ZGlLightModel_GL_RNR_H

#include <Glasses/ZGlLightModel.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>

class ZGlLightModel_GL_Rnr : public ZGlass_GL_Rnr {
private:
  void _init();

protected:
  ZGlLightModel*	mZGlLightModel;

public:
  ZGlLightModel_GL_Rnr(ZGlLightModel* idol) : ZGlass_GL_Rnr(idol), mZGlLightModel(idol) {}

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass ZGlLightModel_GL_Rnr

#endif
