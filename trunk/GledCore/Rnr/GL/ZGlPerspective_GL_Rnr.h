// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlPerspective_GL_RNR_H
#define GledCore_ZGlPerspective_GL_RNR_H

#include <Glasses/ZGlPerspective.h>
#include <Rnr/GL/ZRnrModBase_GL_Rnr.h>

class ZGlPerspective_GL_Rnr : public ZRnrModBase_GL_Rnr {
private:
  void _init();

protected:
  ZGlPerspective*	mZGlPerspective;

  void setup_matrices(RnrDriver* rd, bool push_p);

public:
  ZGlPerspective_GL_Rnr(ZGlPerspective* idol) :
    ZRnrModBase_GL_Rnr(idol), mZGlPerspective(idol)
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass ZGlPerspective_GL_Rnr

#endif
