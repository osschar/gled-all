// $Header$
#include "GLRnrDriver.h"

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// GLRnrDriver
//


GLRnrDriver::GLRnrDriver(Eye* e, const string& r) : RnrDriver(e, r)
{
  mMaxLamps = 8;
  mLamps = new (A_Rnr*)[mMaxLamps];

  mMaxClipPlanes = 6;
  mClipPlanes = new (A_Rnr*)[mMaxClipPlanes];

  bMarkupNodes = false;
}

GLRnrDriver::~GLRnrDriver()
{
  delete [] mLamps;
  delete [] mClipPlanes;
}

/**************************************************************************/

void GLRnrDriver::BeginRender()
{
  RnrDriver::BeginRender();

  for(int i=0; i<mMaxLamps; ++i) {
    mLamps[i] = 0;
  } 
  for(int i=0; i<mMaxClipPlanes; ++i) {
    mClipPlanes[i] = 0;
  }
  bInDLRebuild = false;
}


void GLRnrDriver::EndRender()
{
  for(int l=0; l<mMaxLamps; ++l) {
    if(mLamps[l] != 0) {
      // cout <<"RnrDriver cleaning-up a dirty lamp ...\n";
      mLamps[l]->CleanUp(this);
    }
  }
  for(int l=0; l<mMaxClipPlanes; ++l) {
    if(mClipPlanes[l] != 0) {
      cout <<"RnrDriver cleaning-up a dirty clip-plane ...\n";
      mClipPlanes[l]->CleanUp(this);
    }
  }

  RnrDriver::EndRender();
}

/**************************************************************************/

Int_t GLRnrDriver::GetLamp(A_Rnr* rnr)
{
  for(Int_t i=0; i<mMaxLamps; ++i) {
    if(mLamps[i] == 0) { mLamps[i] = rnr; return i; }
  }
  return -1;
}

void GLRnrDriver::ReturnLamp(Int_t lamp)
{
  mLamps[lamp] = 0;
}

/**************************************************************************/

Int_t GLRnrDriver::GetClipPlane(A_Rnr* rnr)
{
  for(Int_t i=0; i<mMaxClipPlanes; ++i) {
    if(mClipPlanes[i] == 0) { mClipPlanes[i] = rnr; return i; }
  }
  return -1;
}

void GLRnrDriver::ReturnClipPlane(Int_t clip)
{
  mClipPlanes[clip] = 0;
}
