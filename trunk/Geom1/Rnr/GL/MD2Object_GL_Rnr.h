// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_MD2Object_GL_RNR_H
#define Geom1_MD2Object_GL_RNR_H

#include <Glasses/MD2Object.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class MD2Object_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  MD2Object*	mMD2Object;

public:
  MD2Object_GL_Rnr(MD2Object* idol) :
    ZNode_GL_Rnr(idol), mMD2Object(idol)
  { _init(); }

  virtual void Draw(RnrDriver* rd);

}; // endclass MD2Object_GL_Rnr

#endif
