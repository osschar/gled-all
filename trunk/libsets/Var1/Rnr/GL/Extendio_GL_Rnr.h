// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Extendio_GL_RNR_H
#define Var1_Extendio_GL_RNR_H

#include <Glasses/Extendio.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>

class Extendio_GL_Rnr : public ZGlass_GL_Rnr
{
private:
  void _init();

protected:
  Extendio*	mExtendio;

public:
  Extendio_GL_Rnr(Extendio* idol) :
    ZGlass_GL_Rnr(idol), mExtendio(idol)
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void Render(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

  static void render_ceaabox(const Float_t* x, Float_t f=1);

}; // endclass Extendio_GL_Rnr

#endif
