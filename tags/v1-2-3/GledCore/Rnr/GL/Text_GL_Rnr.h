// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Text_GL_RNR_H
#define GledCore_Text_GL_RNR_H

#include <Glasses/Text.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class Text_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  Text*	mText;

public:
  Text_GL_Rnr(Text* idol) : ZNode_GL_Rnr(idol), mText(idol) {}

  virtual void Draw(RnrDriver* rd);

  virtual void Render(RnrDriver* rd);

}; // endclass Text_GL_Rnr

#endif
