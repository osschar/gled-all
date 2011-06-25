// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "CmsXrdSite.h"
#include "CmsXrdSite.c7"

// CmsXrdSite

//______________________________________________________________________________
//
//

ClassImp(CmsXrdSite);

//==============================================================================

void CmsXrdSite::_init()
{}

CmsXrdSite::CmsXrdSite(const Text_t* n, const Text_t* t) :
  ZNameMap(n, t)
{
  _init();
}

CmsXrdSite::~CmsXrdSite()
{}

//==============================================================================
