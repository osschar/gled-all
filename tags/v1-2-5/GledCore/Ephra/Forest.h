// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Forest
#define Gled_Forest

class ZNode;
class Saturn;
#include <Glasses/ZList.h>

#include "TNamed.h"

// Really ... don't need Forest as it is now ...
// Should be more like collection of parent nodes with tags for Frozen/Loadable
// ZNode would then need ZNode* mSuperParent
// ZParent would be Tree spawning ZNode ... as one you can open ForestView on ...

#include <map>

class TFile;

class Forest : public TNamed {
private:
  ZList		mDanglers;
  Saturn*		mSaturn;	//!
public:
  Forest();
  Forest(Saturn* s);
  Forest(Text_t* name, Text_t* title=0);
  Forest(Saturn* s, Text_t* name, Text_t* title=0);
  void		Init();
  void		SetSaturn(Saturn* s) { mSaturn = s; }

  ClassDef(Forest, 0)
}; // endclass Forest

#endif
