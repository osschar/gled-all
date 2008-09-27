// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_UINodeLink_GL_RNR_H
#define Alice_UINodeLink_GL_RNR_H

#include <Glasses/UINodeLink.h>
#include <Rnr/GL/ZNodeLink_GL_Rnr.h>

class UINodeLink_GL_Rnr : public ZNodeLink_GL_Rnr {
 private:
  void _init();

 protected:
  UINodeLink*	mUINodeLink;
  Bool_t	bBelowMouse;

 public:
  UINodeLink_GL_Rnr(UINodeLink* idol) :
    ZNodeLink_GL_Rnr(idol), mUINodeLink(idol)
  { _init();}

  virtual int  Handle(RnrDriver* rd, Fl_Event& ev);

}; // endclass UINodeLink_GL_Rnr

#endif
