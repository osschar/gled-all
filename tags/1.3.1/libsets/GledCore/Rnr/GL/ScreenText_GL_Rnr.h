// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ScreenText_GL_RNR_H
#define GledCore_ScreenText_GL_RNR_H

#include <Glasses/ScreenText.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>

class ScreenText_GL_Rnr : public ZGlass_GL_Rnr {
private:
  void _init();

protected:
  ScreenText*	mScreenText;

public:
  ScreenText_GL_Rnr(ScreenText* idol) :
    ZGlass_GL_Rnr(idol), mScreenText(idol)
  { _init(); }

  //virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  //virtual void PostDraw(RnrDriver* rd);

}; // endclass ScreenText_GL_Rnr

#endif
