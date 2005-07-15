// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TOFSector
//
//

#include "TOFSector.h"
#include "TOFSector.c7"

ClassImp(TOFSector)

/**************************************************************************/

void TOFSector::_init()
{
  // Override settings from ZGlass
  bUseDispList = true;

  // *** Set all links to 0 *** 
  mInfo = 0;
  mSecID = 0;
}

/**************************************************************************/
TOFSector::TOFSector(Int_t sec, TOFDigitsInfo* info) : ZNode() 
{
  _init();
  mSecID = sec;
  mInfo = info;
  if(mInfo) mInfo->IncRefCount();

  SetName(GForm("TOF Sec %d",mSecID));
}

/**************************************************************************/
