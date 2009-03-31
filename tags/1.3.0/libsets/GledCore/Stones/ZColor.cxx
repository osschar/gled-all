// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZColor.h"

ClassImp(ZColor);

ostream& operator<<(ostream& s, ZColor& c)
{
  return s <<c[0]<<","<<c[1]<<","<<c[2]<<","<<c[3];
}
