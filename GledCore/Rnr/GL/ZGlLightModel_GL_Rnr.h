// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlLightModel_GL_RNR_H
#define GledCore_ZGlLightModel_GL_RNR_H

#include <Glasses/ZGlLightModel.h>
#include <Rnr/GL/ZRnrModBase_GL_Rnr.h>

class ZGlLightModel_GL_Rnr : public ZRnrModBase_GL_Rnr {
private:
  void _init();

protected:
  ZGlLightModel*	mZGlLightModel;

public:
  ZGlLightModel_GL_Rnr(ZGlLightModel* idol) : ZRnrModBase_GL_Rnr(idol), mZGlLightModel(idol) {}

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

  void SetupGL();

}; // endclass ZGlLightModel_GL_Rnr

#endif
