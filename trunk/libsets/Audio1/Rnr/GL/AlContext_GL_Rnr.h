// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Audio1_AlContext_GL_RNR_H
#define Audio1_AlContext_GL_RNR_H

#include <Glasses/AlContext.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class AlContext_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  AlContext*	mAlContext;

public:
  AlContext_GL_Rnr(AlContext* idol) :
    ZNode_GL_Rnr(idol), mAlContext(idol)
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass AlContext_GL_Rnr

#endif
