// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GlobalLamps_GL_RNR_H
#define GledCore_GlobalLamps_GL_RNR_H

#include <Glasses/GlobalLamps.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>

class Lamp_GL_Rnr;

class GlobalLamps_GL_Rnr : public ZGlass_GL_Rnr
{
private:
  void _init();

protected:
  GlobalLamps*		mGlobalLamps;
  list<Lamp_GL_Rnr*>	mLampsOn;

public:
  GlobalLamps_GL_Rnr(GlobalLamps* idol) :
    ZGlass_GL_Rnr(idol), mGlobalLamps(idol) { _init(); }
  virtual ~GlobalLamps_GL_Rnr() {}

  virtual void PreDraw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass GlobalLamps_GL_Rnr

#endif
