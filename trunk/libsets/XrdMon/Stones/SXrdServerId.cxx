// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SXrdServerId.h"

// SXrdServerId

//______________________________________________________________________________
//
//

ClassImp(SXrdServerId);


void SXrdServerId::Clear()
{
  ip4 = 0;
  stod = 0;
  port = 0;
}
