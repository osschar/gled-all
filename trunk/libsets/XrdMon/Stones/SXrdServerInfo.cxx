// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SXrdServerInfo.h"
#include "Glasses/XrdServer.h"

// SXrdServerInfo

//______________________________________________________________________________
//
//

ClassImp(SXrdServerInfo);

//==============================================================================

void SXrdServerInfo::Assign(const XrdServer* s)
{
  mHost   = s->RefHost();
  mDomain = s->RefDomain();
  mSite   = s->RefSite();
}
