// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Audio1_AlListener_GL_RNR_H
#define Audio1_AlListener_GL_RNR_H

#include <Glasses/AlListener.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class AlListener_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  AlListener*	mAlListener;

public:
  AlListener_GL_Rnr(AlListener* idol) :
    ZNode_GL_Rnr(idol), mAlListener(idol)
  { _init(); }

  //virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  //virtual void PostDraw(RnrDriver* rd);

}; // endclass AlListener_GL_Rnr

#endif
