// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "EyeInfo.h"
#include <Net/Ray.h>
#include <Stones/ZMIR.h>
#include <Ephra/Saturn.h>

ClassImp(EyeInfo)

EyeInfo::EyeInfo(const Text_t* n, const Text_t* t) : ZGlass(n,t) {
  hSocket = 0; mMaster = 0;
}

/**************************************************************************/

void EyeInfo::Message(const Text_t* s)
{
  if(mMir == 0) return;
  Ray r(Ray::RQN_message, mMir->Caller, s);
  mSaturn->SingleRay(this, r);
}

void EyeInfo::Error(const Text_t* s)
{
  if(mMir == 0) return;
  Ray r(Ray::RQN_error, mMir->Caller, s);
  mSaturn->SingleRay(this, r);
}

/**************************************************************************/

#include "EyeInfo.c7"
