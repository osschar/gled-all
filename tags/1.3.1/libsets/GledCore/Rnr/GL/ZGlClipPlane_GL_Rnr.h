// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlClipPlane_GL_RNR_H
#define GledCore_ZGlClipPlane_GL_RNR_H

#include <Glasses/ZGlClipPlane.h>
#include <Rnr/GL/ZRnrModBase_GL_Rnr.h>

class ZGlClipPlane_GL_Rnr : public ZRnrModBase_GL_Rnr {
private:
  void _init();

protected:
  ZGlClipPlane*	mZGlClipPlane;
  Int_t         mClipId;

public:
  ZGlClipPlane_GL_Rnr(ZGlClipPlane* idol) :
    ZRnrModBase_GL_Rnr(idol), mZGlClipPlane(idol)
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

  virtual void CleanUp(RnrDriver* rd);

  virtual void TurnOn(RnrDriver* rd);
  virtual void TurnOff(RnrDriver* rd);

  void RnrSelf();

}; // endclass ZGlClipPlane_GL_Rnr

#endif
