// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "STabletPoint.h"

// STabletPoint

//______________________________________________________________________________
//
//

ClassImp(STabletPoint);

//==============================================================================

void STabletPoint::Print() const
{
  printf("%8.3lf %8.3lf %8.3lf, %8.3lf, %8.3lf\n", x, y, z, t, p); 
}
