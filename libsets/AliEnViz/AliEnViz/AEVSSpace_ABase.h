// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVSSpace_ABase_H
#define AliEnViz_AEVSSpace_ABase_H

#include <Rtypes.h>

class AEVSite;

class AEVSSpace_ABase
{
public:
  virtual ~AEVSSpace_ABase() {}

  virtual Bool_t ImportSite(AEVSite* site, Bool_t warn=false) = 0;
  virtual void   ClearSiteVizes() {}

  ClassDef(AEVSSpace_ABase, 0); // Abstract interface - site-management API for space representations.
}; // endclass AEVSSpace_ABase

#endif
