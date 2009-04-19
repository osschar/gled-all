// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_LandMark_GL_RNR_H
#define Var1_LandMark_GL_RNR_H

#include <Glasses/LandMark.h>
#include <Rnr/GL/Extendio_GL_Rnr.h>

class LandMark_GL_Rnr : public Extendio_GL_Rnr
{
private:
  void _init();

protected:
  LandMark*	mLandMark;

public:
  LandMark_GL_Rnr(LandMark* idol) :
    Extendio_GL_Rnr(idol), mLandMark(idol)
  { _init(); }

  //virtual void PreDraw(RnrDriver* rd);
  //virtual void Draw(RnrDriver* rd);
  virtual void Render(RnrDriver* rd);
  //virtual void PostDraw(RnrDriver* rd);

}; // endclass LandMark_GL_Rnr

#endif
