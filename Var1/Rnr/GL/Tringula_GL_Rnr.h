// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Tringula_GL_RNR_H
#define Var1_Tringula_GL_RNR_H

#include <Glasses/Tringula.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

#include <GL/glu.h>

class Tringula_GL_Rnr : public ZNode_GL_Rnr
{
private:
  void _init();

protected:
  Tringula*	 mTringula;
  GLUquadricObj* mQuadric;
  TimeStamp_t    mMeshTringStamp;

public:
  Tringula_GL_Rnr(Tringula* idol) :
    ZNode_GL_Rnr(idol), mTringula(idol)
  { _init(); }
  virtual ~Tringula_GL_Rnr();

  void RenderExtendio(RnrDriver* rd, Extendio* dyno);
  void RenderExtendios(RnrDriver* rd, AList* list);

  virtual void Draw(RnrDriver* rd);
  virtual void Render(RnrDriver* rd);

}; // endclass Tringula_GL_Rnr

#endif
