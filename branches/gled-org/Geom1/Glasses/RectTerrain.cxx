// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// RectTerrain
//
// A rectangular mNx * mNy height field.
// mO(x|y) ~ origin, mD(x|y) ~ grid spacing.
// Can be set from a ZImage via void SetFromImage(ZImage* image).
//
// Renderer is stupid, so expect it to be slow for large grids (512x512).

#include "RectTerrain.h"
#include <Glasses/ZImage.h>

#include <IL/il.h>

#include <stdio.h>

ClassImp(RectTerrain)

void RectTerrain::_init()
{
  mNx = mNy = 0;
  mOx = mOy = 0;
  mDx = mDy = 1;
  mZScale = 1; mMinZ = 0; mMaxZ = 0;
  mMinCol.gray(0.2); mMaxCol.gray(1);
  mColSep = 1;
  bStudySize = false;
}

/**************************************************************************/

void RectTerrain::ToStd()
{
  mOx = mOy = 0;
  Stamp(LibID(), ClassID());
}

void RectTerrain::ToCenter()
{
  mOx = -0.5 * mDx * mNx;
  mOy = -0.5 * mDy * mNy;
  Stamp(LibID(), ClassID());
}

/**************************************************************************/

void RectTerrain::SetFromImage(ZImage* image)
{
  ZImage::sILMutex.Lock();
  image->bind();
  int w = image->w(), h = image->h();
  ILushort* data = new ILushort[w*h];
  ilCopyPixels(0,0,0, w,h,1, IL_LUMINANCE,IL_UNSIGNED_SHORT, data);
  ZImage::sILMutex.Unlock();

  mNx = w; mNy = h;
  mP.ResizeTo(mNx, mNy);
  mMinZ = 1e9; mMaxZ = -1e9;

  for(UInt_t y=0; y<mNy; ++y) {
    ILushort* bar = &(data[(mNy-y-1)*w]);
    for(UInt_t x=0; x<mNx; ++x) {
      const float z = (float)(*(bar++));
      if(z > mMaxZ) mMaxZ = z;
      if(z < mMinZ) mMinZ = z;
      mP(x,y) = z;
    }
  }
  delete [] data;

  float zoffset = 0; // set z = 0 at 0
  float zfac = 1;    // scale not

  float delta = 1.001*(mMaxZ - mMinZ);
  for(UInt_t x=0; x<mNx; ++x) {
    for(UInt_t y=0; y<mNy; ++y) {
      mP(x,y) = ( (mP(x,y) - mMinZ)/delta - zoffset ) * zfac;
    }
  }
  
  if(zfac > 0) {
    mMinZ = -zoffset * zfac; mMaxZ = (1 - zoffset) * zfac;
  } else {
    mMaxZ = -zoffset * zfac; mMinZ = (1 - zoffset) * zfac;
  }

  Stamp(LibID(), ClassID()); 
}

/**************************************************************************/

#include "RectTerrain.c7"
