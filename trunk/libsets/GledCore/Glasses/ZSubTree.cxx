// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZSubTree.h"
#include "ZSubTree.c7"

ClassImp(ZSubTree);

void ZSubTree::_init()
{
  mRoot = 0;
  mDepth = 1; bFollowLinks = true; bFollowLists = true;
}

/**************************************************************************/
