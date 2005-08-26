// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_WGlValuator_GL_RNR_H
#define GledCore_WGlValuator_GL_RNR_H

#include <Glasses/WGlValuator.h>
#include <Glasses/WGlFrameStyle.h>
#include <Glasses/ZRlFont.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class WGlValuator_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  WGlValuator*	mWGlValuator;

  RnrModStore	mFontRMS;
  RnrModStore	mFrameRMS;

  Bool_t	bBelowMouse;

  Int_t         mX, mY;

  Double_t get_value();

public:
  WGlValuator_GL_Rnr(WGlValuator* idol) :
    ZNode_GL_Rnr(idol), mWGlValuator(idol),
    mFontRMS(ZRlFont::FID()), mFrameRMS(WGlFrameStyle::FID())
  { _init(); }

  virtual void Draw(RnrDriver* rd);

  virtual int  Handle(RnrDriver* rd, Fl_Event& ev);

}; // endclass WGlValuator_GL_Rnr

#endif
