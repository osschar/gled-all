// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Rect.h"

ClassImp(Rect)

void Rect::_init() {
  mULen = mVLen = 1;
  mUStrips = mVStrips = 10;
  mWidth = 1;
}

#include "Rect.c7"