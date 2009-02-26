// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// GLRnrDriver
//

#include "GLRnrDriver.h"
#include <Glasses/ZGlColorFader.h>
#include <GL/glew.h>

GLRnrDriver::GLRnrDriver(Eye* e, const TString& r) : RnrDriver(e, r)
{
  bInPicking = false;
  bDoPickOps = false;
  mPickCount = 0;
  mPickSize  = 1024;
  mPickVector.resize(mPickSize);

  mBelowMouse = mPushed = mFocus = 0;

  mMaxLamps = 8;
  mLamps = new A_Rnr* [mMaxLamps];

  mMaxClipPlanes = 6;
  mClipPlanes = new A_Rnr* [mMaxClipPlanes];

  bMarkupNodes = false;

  bRedraw = false;

  mFaderStack = 0;
}

GLRnrDriver::~GLRnrDriver()
{
  delete [] mLamps;
  delete [] mClipPlanes;
}

/**************************************************************************/
// Rendering
/**************************************************************************/

void GLRnrDriver::BeginRender()
{
  RnrDriver::BeginRender();

  if(mFaderStack == 0) {
    // Make default rnrmod.
    mFaderStack = &mRMStacks.insert(make_pair(ZGlColorFader::FID(), RMStack())).first->second;
    ZGlass* lens = GledNS::ConstructLens(ZGlColorFader::FID());
    A_Rnr*  rnr  = lens->VGlassInfo()->SpawnRnr(mRnrName, lens);
    mFaderStack->m_def_autogen = new RnrMod(lens, rnr);
  }

  bRedraw = false;

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
// Name stack / Picking
/**************************************************************************/

void GLRnrDriver::BeginPick()
{
  bInPicking = true;
  bDoPickOps = true;
  mPickCount = 0;
  mStackCopy.clear();
}

void GLRnrDriver::EndPick()
{
  bInPicking = false;
}

/**************************************************************************/

void GLRnrDriver::ClearNameStack()
{
  if(!bInPicking) return;
  int m = mStackCopy.size();
  while(m-- > 0)
    glPopName();
}

void GLRnrDriver::RestoreNameStack()
{
  if(!bInPicking) return;
  list<UInt_t>::iterator i = mStackCopy.begin();
  while(i != mStackCopy.end())
    glPushName(*(i++));
}

/**************************************************************************/

void GLRnrDriver::push_name(A_Rnr* rnr, void* ud)
{
  ++mPickCount;
  if(mPickCount >= mPickSize) {
    mPickSize *= 2;
    mPickVector.resize(mPickSize);
  }
  A_Rnr::NSE_t& nse(mPickVector[mPickCount]);
  nse.fRnr      = rnr;
  nse.fUserData = ud;
  glPushName(mPickCount);
  mStackCopy.push_back(mPickCount);
}

void GLRnrDriver::pop_name()
{
  mStackCopy.pop_back();
  glPopName();
}

/**************************************************************************/
// Lamps & Clipping planes
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

/**************************************************************************/
// Color, PointSize and LineWidth scaling
/**************************************************************************/

Float_t GLRnrDriver::Color(Float_t r, Float_t g, Float_t b, Float_t a)
{
  const ZColor& C = ColorFader().RefColorFac();
  Float_t alpha = a*C.a();
  glColor4f(r*C.r(), g*C.g(), b*C.b(), alpha);
  return alpha;
}

Float_t GLRnrDriver::PointSize(Float_t size)
{
  if(size == 0)
    return -1;
  Float_t s = size*ColorFader().GetPointSizeFac();
  glPointSize(s);
  return s;
}

Float_t GLRnrDriver::LineWidth(Float_t width)
{
  if(width == 0)
    return -1;
  Float_t w = width*ColorFader().GetLineWidthFac();
  glLineWidth(w);
  return w;
}

