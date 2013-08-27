// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlBlending_GL_RNR_H
#define GledCore_ZGlBlending_GL_RNR_H

#include <Glasses/ZGlBlending.h>
#include <Rnr/GL/ZRnrModBase_GL_Rnr.h>

class ZGlBlending_GL_Rnr : public ZRnrModBase_GL_Rnr {
private:
  void _init();

protected:
  ZGlBlending*	mZGlBlending;

public:
  ZGlBlending_GL_Rnr(ZGlBlending* idol) : ZRnrModBase_GL_Rnr(idol), mZGlBlending(idol) {}

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

  void SetupGL(RnrDriver* rd);

}; // endclass ZGlBlending_GL_Rnr

#endif
