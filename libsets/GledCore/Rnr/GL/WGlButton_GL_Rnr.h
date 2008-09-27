// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_WGlButton_GL_RNR_H
#define GledCore_WGlButton_GL_RNR_H

#include <Glasses/WGlButton.h>
#include <Glasses/WGlFrameStyle.h>
#include <Glasses/ZRlFont.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class WGlButton_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  WGlButton*	mWGlButton;
  RnrModStore	mFontRMS;
  RnrModStore	mFrameRMS;

  Bool_t	bBelowMouse;

public:
  WGlButton_GL_Rnr(WGlButton* idol) :
    ZNode_GL_Rnr(idol), mWGlButton(idol),
    mFontRMS(ZRlFont::FID()), mFrameRMS(WGlFrameStyle::FID())
  { _init(); }

  virtual void Draw(RnrDriver* rd);

  virtual int  Handle(RnrDriver* rd, Fl_Event& ev);


}; // endclass WGlButton_GL_Rnr

#endif
