// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVEventBatch_GL_RNR_H
#define AliEnViz_AEVEventBatch_GL_RNR_H

#include <Glasses/AEVEventBatch.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class AEVEventBatch_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  AEVEventBatch*	mAEVEventBatch;

public:
  AEVEventBatch_GL_Rnr(AEVEventBatch* idol) : ZNode_GL_Rnr(idol), mAEVEventBatch(idol) { _init(); }

  virtual void Draw(RnrDriver* rd);

}; // endclass AEVEventBatch_GL_Rnr

#endif
