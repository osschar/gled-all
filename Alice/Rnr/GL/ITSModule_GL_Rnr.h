// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_ITSModule_GL_RNR_H
#define Alice_ITSModule_GL_RNR_H

#include <Glasses/ITSModule.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class ITSModule_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  ITSModule*	mITSModule;

public:
  ITSModule_GL_Rnr(ITSModule* idol) :
    ZNode_GL_Rnr(idol), mITSModule(idol)
  { _init(); }

  virtual void Draw(RnrDriver* rd);

}; // endclass ITSModule_GL_Rnr

#endif
