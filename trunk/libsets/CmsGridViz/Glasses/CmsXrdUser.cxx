// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "CmsXrdUser.h"
#include "CmsXrdUser.c7"

// CmsXrdUser

//______________________________________________________________________________
//
//

ClassImp(CmsXrdUser);

//==============================================================================

void CmsXrdUser::_init()
{}

CmsXrdUser::CmsXrdUser(const Text_t* n, const Text_t* t) :
  ZList(n, t)
{
  _init();
}

CmsXrdUser::~CmsXrdUser()
{}

//==============================================================================
