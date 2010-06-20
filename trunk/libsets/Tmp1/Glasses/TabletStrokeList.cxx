// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TabletStrokeList.h"
#include "TabletStrokeList.c7"

// TabletStrokeList

//______________________________________________________________________________
//
//

ClassImp(TabletStrokeList);

//==============================================================================

void TabletStrokeList::_init()
{}

TabletStrokeList::TabletStrokeList(const Text_t* n, const Text_t* t) :
  ZNode(n, t)
{
  _init();
}

TabletStrokeList::~TabletStrokeList()
{}

//==============================================================================
