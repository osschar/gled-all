// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZRnrModBase
//
//

#include "ZRnrModBase.h"
#include "ZRnrModBase.c7"

ClassImp(ZRnrModBase);

/**************************************************************************/

void ZRnrModBase::_init()
{
  // From ZGlass:
  bUseNameStack = false;
}
