// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GTS_GTSurf_GL_RNR_H
#define GTS_GTSurf_GL_RNR_H

#include <Glasses/GTSurf.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class GTSurf_GL_Rnr : public ZNode_GL_Rnr {
private:

protected:
  GTSurf*	mGTSurf;

public:
  GTSurf_GL_Rnr(GTSurf* idol) : ZNode_GL_Rnr(idol), mGTSurf(idol) {}

  virtual void Draw(RnrDriver* rd);

}; // endclass GTSurf_GL_Rnr

#endif
