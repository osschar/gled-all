// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVCompElRep_GL_RNR_H
#define AliEnViz_AEVCompElRep_GL_RNR_H

#include <Glasses/AEVCompElRep.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class AEVCompElRep_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  AEVCompElRep*	mAEVCompElRep;

public:
  AEVCompElRep_GL_Rnr(AEVCompElRep* idol) : ZNode_GL_Rnr(idol), mAEVCompElRep(idol) { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass AEVCompElRep_GL_Rnr

#endif
