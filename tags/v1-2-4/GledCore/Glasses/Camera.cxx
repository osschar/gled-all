// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include <Glasses/Camera.h>

ClassImp(Camera)

void Camera::Home() { mTrans.UnitMatrix(); }

#include "Camera.c7"
