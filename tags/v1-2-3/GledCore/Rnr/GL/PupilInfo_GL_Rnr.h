// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_PupilInfo_GL_RNR_H
#define Gled_PupilInfo_GL_RNR_H

#include <Glasses/PupilInfo.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>

class PupilInfo_GL_Rnr : public ZGlass_GL_Rnr {
private:

protected:
  PupilInfo*	mPupilInfo;

public:
  PupilInfo_GL_Rnr(PupilInfo* idol) : ZGlass_GL_Rnr(idol), mPupilInfo(idol) {}

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass PupilInfo_GL_Rnr

#endif
