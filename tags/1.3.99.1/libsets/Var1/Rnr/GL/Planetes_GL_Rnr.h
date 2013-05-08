// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Planetes_GL_RNR_H
#define Var1_Planetes_GL_RNR_H

#include <Glasses/Planetes.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class Planetes_GL_Rnr : public ZNode_GL_Rnr
{
private:
  void _init();

protected:
  Planetes*	mPlanetes;

public:
  Planetes_GL_Rnr(Planetes* idol);
  virtual ~Planetes_GL_Rnr();

  // virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  // virtual void PostDraw(RnrDriver* rd);

  virtual void Render(RnrDriver* rd);

}; // endclass Planetes_GL_Rnr

#endif
