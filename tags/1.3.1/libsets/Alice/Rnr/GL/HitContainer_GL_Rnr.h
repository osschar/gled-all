// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_HitContainer_GL_RNR_H
#define Alice_HitContainer_GL_RNR_H

#include <Glasses/HitContainer.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class HitContainer_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  HitContainer*	mHitContainer;

public:
  HitContainer_GL_Rnr(HitContainer* idol) :
    ZNode_GL_Rnr(idol), mHitContainer(idol)
  { _init(); }

 
  virtual void Draw(RnrDriver* rd);

}; // endclass HitContainer_GL_Rnr

#endif
