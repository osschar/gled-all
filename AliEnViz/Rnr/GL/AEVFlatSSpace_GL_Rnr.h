// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVFlatSSpace_GL_RNR_H
#define AliEnViz_AEVFlatSSpace_GL_RNR_H

#include <Glasses/AEVFlatSSpace.h>
#include <Rnr/GL/Board_GL_Rnr.h>

class AEVFlatSSpace_GL_Rnr : public Board_GL_Rnr {
private:
  void _init();

protected:
  AEVFlatSSpace*	mAEVFlatSSpace;

public:
  AEVFlatSSpace_GL_Rnr(AEVFlatSSpace* idol) : Board_GL_Rnr(idol), mAEVFlatSSpace(idol) { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass AEVFlatSSpace_GL_Rnr

#endif
