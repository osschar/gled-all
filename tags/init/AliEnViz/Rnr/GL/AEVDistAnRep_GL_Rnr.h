// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVDistAnRep_GL_RNR_H
#define AliEnViz_AEVDistAnRep_GL_RNR_H

#include <Glasses/AEVDistAnRep.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class AEVDistAnRep_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  AEVDistAnRep*	mAEVDistAnRep;

public:
  AEVDistAnRep_GL_Rnr(AEVDistAnRep* idol) : ZNode_GL_Rnr(idol), mAEVDistAnRep(idol) { _init(); }

  // virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  // virtual void PostDraw(RnrDriver* rd);

}; // endclass AEVDistAnRep_GL_Rnr

#endif
