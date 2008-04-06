// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_PSMark_GL_RNR_H
#define Var1_PSMark_GL_RNR_H

#include <Glasses/PSMark.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class PSMark_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  PSMark*	mPSMark;

public:
  PSMark_GL_Rnr(PSMark* idol) :
    ZNode_GL_Rnr(idol), mPSMark(idol)
  { _init(); }

  virtual void Render(RnrDriver* rd);

}; // endclass PSMark_GL_Rnr

#endif
