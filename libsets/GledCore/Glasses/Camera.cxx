// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include <Glasses/Camera.h>
#include "Camera.c7"

ClassImp(Camera);

//==============================================================================

void Camera::Home()
{
  mTrans = mHomeTrans;
  Stamp(FID());
}

void Camera::Identity()
{
  mTrans.UnitTrans();
  Stamp(FID());
}

//==============================================================================

void Camera::SetHomeTrans()
{
  mHomeTrans = mTrans;
}

void Camera::ResetHomeTrans()
{
  mHomeTrans.UnitTrans();
}
