// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_GlobalLamps_GL_RNR_H
#define Gled_GlobalLamps_GL_RNR_H

#include <Glasses/GlobalLamps.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>

class GlobalLamps_GL_Rnr : public ZGlass_GL_Rnr {
private:

protected:
  GlobalLamps*		mGlobalLamps;
  list<Lamp_GL_Rnr*>	mLampsOn;

public:
  GlobalLamps_GL_Rnr(GlobalLamps* idol) : ZGlass_GL_Rnr(idol), mGlobalLamps(idol) {}
  virtual ~GlobalLamps_GL_Rnr() {}

  virtual void PreDraw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass GlobalLamps_GL_Rnr

#endif
