// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Numerica_Spheror_GL_RNR_H
#define Numerica_Spheror_GL_RNR_H

#include <Glasses/Spheror.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class Spheror_GL_Rnr : public ZNode_GL_Rnr {
private:

protected:
  Spheror*	mSpheror;

public:
  Spheror_GL_Rnr(Spheror* idol) : ZNode_GL_Rnr(idol), mSpheror(idol) {}

  virtual void Draw(RnrDriver* rd);

}; // endclass Spheror_GL_Rnr

#endif
