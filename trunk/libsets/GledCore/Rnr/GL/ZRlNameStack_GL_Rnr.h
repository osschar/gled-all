// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZRlNameStack_GL_RNR_H
#define GledCore_ZRlNameStack_GL_RNR_H

#include <Glasses/ZRlNameStack.h>
#include <Rnr/GL/ZRnrModBase_GL_Rnr.h>

class ZRlNameStack_GL_Rnr : public ZRnrModBase_GL_Rnr {
private:
  void _init();

protected:
  bool          bExState;
  ZRlNameStack*	mZRlNameStack;

public:
  ZRlNameStack_GL_Rnr(ZRlNameStack* idol) :
    ZRnrModBase_GL_Rnr(idol), mZRlNameStack(idol)
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass ZRlNameStack_GL_Rnr

#endif
