// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_ITSScaledModule_H
#define Alice_ITSScaledModule_H

#include <Glasses/ITSModule.h>

class ITSScaledModule : public ITSModule {
  MAC_RNR_FRIENDS(ITSScaledModule);

private:
  void _init();

protected:

public:
  ITSScaledModule(const Text_t* n="ITSScaledModule", const Text_t* t=0) :
    ITSModule(n,t) { _init(); }

  ITSScaledModule(Int_t id, ITSDigitsInfo* info):ITSModule(id, info){_init();}

#include "ITSScaledModule.h7"
  ClassDef(ITSScaledModule, 1)
}; // endclass ITSScaledModule

GlassIODef(ITSScaledModule);

#endif
