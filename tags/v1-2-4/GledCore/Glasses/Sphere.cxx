// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Sphere.h"

void Sphere::_init()
{
  mLOD = 8;
}

ClassImp(Sphere)
#include "Sphere.c7"
