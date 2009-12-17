// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TringuRep_GL_RNR_H
#define Var1_TringuRep_GL_RNR_H

#include <Glasses/TringuRep.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class TringuRep_GL_Rnr : public ZNode_GL_Rnr
{
private:
  void _init();

protected:
  TringuRep*	mTringuRep;

public:
  TringuRep_GL_Rnr(TringuRep* idol);
  virtual ~TringuRep_GL_Rnr();

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

  virtual void Render(RnrDriver* rd);

}; // endclass TringuRep_GL_Rnr

#endif
