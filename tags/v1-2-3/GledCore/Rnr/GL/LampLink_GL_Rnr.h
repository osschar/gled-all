// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_LampLink_GL_RNR_H
#define Gled_LampLink_GL_RNR_H

#include <Glasses/LampLink.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>

class LampLink_GL_Rnr : public ZGlass_GL_Rnr {
private:

protected:
  LampLink*	mLampLink;

public:
  LampLink_GL_Rnr(LampLink* idol) : ZGlass_GL_Rnr(idol), mLampLink(idol) {}

  virtual void Draw(RnrDriver* rd);

}; // endclass LampLink_GL_Rnr

#endif
