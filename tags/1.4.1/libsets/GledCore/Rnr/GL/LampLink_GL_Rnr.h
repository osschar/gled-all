// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_LampLink_GL_RNR_H
#define GledCore_LampLink_GL_RNR_H

#include <Glasses/LampLink.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>

class LampLink_GL_Rnr : public ZGlass_GL_Rnr {
private:
  void _init();

protected:
  Bool_t        bWarn;
  LampLink*	mLampLink;

public:
  LampLink_GL_Rnr(LampLink* idol) : ZGlass_GL_Rnr(idol), mLampLink(idol) { _init(); }

  virtual void AbsorbRay(Ray& ray);

  virtual void Draw(RnrDriver* rd);

}; // endclass LampLink_GL_Rnr

#endif
