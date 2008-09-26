// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVSphereSSpace_GL_RNR_H
#define AliEnViz_AEVSphereSSpace_GL_RNR_H

#include <Glasses/AEVSphereSSpace.h>
#include <Rnr/GL/SMorph_GL_Rnr.h>

class AEVSphereSSpace_GL_Rnr : public SMorph_GL_Rnr {
private:
  void _init();

protected:
  AEVSphereSSpace*	mAEVSphereSSpace;

public:
  AEVSphereSSpace_GL_Rnr(AEVSphereSSpace* idol) : SMorph_GL_Rnr(idol), mAEVSphereSSpace(idol) { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass AEVSphereSSpace_GL_Rnr

#endif
