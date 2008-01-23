// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TringuCam_GL_RNR_H
#define Var1_TringuCam_GL_RNR_H

#include <Glasses/TringuCam.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class TringuCam_GL_Rnr : public ZNode_GL_Rnr
{
private:
  void _init();

protected:
  TringuCam*	mTringuCam;

  int		mMouseX, mMouseY;

  TVector3      mClickPos, mClickDir;

public:
  TringuCam_GL_Rnr(TringuCam* idol) :
    ZNode_GL_Rnr(idol), mTringuCam(idol)
  { _init(); }

  virtual void Draw(RnrDriver* rd);

  virtual int  Handle(RnrDriver* rd, Fl_Event& ev);

}; // endclass TringuCam_GL_Rnr

#endif
