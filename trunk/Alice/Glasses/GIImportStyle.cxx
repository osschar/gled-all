// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// GIImportStyle
//
//

#include "GIImportStyle.h"
#include "GIImportStyle.c7"

ClassImp(GIImportStyle)

/**************************************************************************/

  void GIImportStyle::_init()
{
  // *** Set all links to 0 ***
  mImportKine = true;
  mImportHits=true;
  mImportClusters=true;
  mImportRec = true;
  mRnrKine = true;
  mRnrHits = false;
  mRnrClusters = false;
  mRnrRec = false;
}

/**************************************************************************/


/**************************************************************************/
