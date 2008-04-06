// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AEVSiteViz
//
//

#include "AEVSiteViz.h"
#include "AEVSiteViz.c7"

ClassImp(AEVSiteViz)

/**************************************************************************/

void AEVSiteViz::_init()
{
  // ZNode::OM stuffe
  bUseOM = true; mOM = 0;

  mColor.rgba(0.6, 0, 0.6);
}

/**************************************************************************/


/**************************************************************************/
