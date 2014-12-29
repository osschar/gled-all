// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TringulaTester_GL_RNR_H
#define Var1_TringulaTester_GL_RNR_H

#include <Glasses/TringulaTester.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class TringulaTester_GL_Rnr : public ZNode_GL_Rnr
{
private:
  void _init();

protected:
  TringulaTester  *mTringulaTester;
  GLUquadricObj   *mQuadric;

public:
  TringulaTester_GL_Rnr(TringulaTester* idol);
  virtual ~TringulaTester_GL_Rnr();

  virtual void Draw(RnrDriver* rd);

}; // endclass TringulaTester_GL_Rnr

#endif
