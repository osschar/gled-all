// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_SMorph_GL_Rnr_H
#define Geom1_SMorph_GL_Rnr_H

#include <Glasses/SMorph.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class SMorph_GL_Rnr : public ZNode_GL_Rnr {
private:

protected:
  SMorph*	mSMorph;

public:
  SMorph_GL_Rnr(SMorph* m) : ZNode_GL_Rnr(m), mSMorph(m) {}

  virtual void Render(RnrDriver* rd);
  virtual void Triangulate(RnrDriver* rd);

}; // endclass SMorph_GL_Rnr

#endif
