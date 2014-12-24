// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AEVSiteList.h"
#include "AEVSiteList.c7"

#include "AEVSite.h"

// AEVSiteList

//______________________________________________________________________________
//
//

ClassImp(AEVSiteList);

//==============================================================================

void AEVSiteList::_init()
{
  SetElementFID(AEVSite::FID());
}

AEVSiteList::AEVSiteList(const Text_t* n, const Text_t* t) :
  ZNameMap(n, t)
{
  _init();
}

AEVSiteList::~AEVSiteList()
{}

//==============================================================================

AEVSite* AEVSiteList::FindSite(const TString& name)
{
  return dynamic_cast<AEVSite*>(GetElementByName(name));
}
