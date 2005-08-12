// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_ITSScaledModule_GL_RNR_H
#define Alice_ITSScaledModule_GL_RNR_H

#include <Glasses/ITSScaledModule.h>
#include <Rnr/GL/ITSModule_GL_Rnr.h>

class ITSScaledModule_GL_Rnr : public ITSModule_GL_Rnr {
private:
  void _init();

protected:
  ITSScaledModule*	mITSScaledModule;

public:
  ITSScaledModule_GL_Rnr(ITSScaledModule* idol) :
    ITSModule_GL_Rnr(idol), mITSScaledModule(idol)
  { _init(); }

  virtual void Render(RnrDriver* rd);
}; // endclass ITSScaledModule_GL_Rnr

#endif
