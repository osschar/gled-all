// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVMapViz_GL_RNR_H
#define AliEnViz_AEVMapViz_GL_RNR_H

#include <Glasses/AEVMapViz.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class AEVMapViz_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  AEVMapViz*	mAEVMapViz;

public:
  AEVMapViz_GL_Rnr(AEVMapViz* idol) : ZNode_GL_Rnr(idol), mAEVMapViz(idol)
  { _init(); }

  /*
  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);
  */

}; // endclass AEVMapViz_GL_Rnr

#endif
