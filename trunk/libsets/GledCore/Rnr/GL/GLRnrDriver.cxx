// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// GLRnrDriver
//

#include "GLRnrDriver.h"
#include <RnrBase/Fl_Event_Enums.h>
#include <Glasses/ZGlColorFader.h>

#include <TMath.h>

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

  // ----------------------------------------------------------------

  mQuadricStd = 0;
  mQuadricStdNoNormals = 0;
}

GLRnrDriver::~GLRnrDriver()
{
  delete [] mLamps;
  delete [] mClipPlanes;

  if (mQuadricStd != 0)
  {
    gluDeleteQuadric(mQuadricStd);
    gluDeleteQuadric(mQuadricStdNoNormals);
  }
}

/**************************************************************************/
// Rendering
/**************************************************************************/

void GLRnrDriver::BeginRender()
{
  if (mQuadricStd == 0)
  {
    mQuadricStd = gluNewQuadric();
    gluQuadricDrawStyle(mQuadricStd, GLU_FILL);
    gluQuadricNormals(mQuadricStd, GLU_SMOOTH);
    
    mQuadricStdNoNormals = gluNewQuadric();
    gluQuadricDrawStyle(mQuadricStdNoNormals, GLU_FILL);
    gluQuadricNormals(mQuadricStdNoNormals, GLU_NONE);
  }
  
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
// Mouse event handling
/**************************************************************************/

void GLRnrDriver::PreEventHandling(A_Rnr::Fl_Event& e)
{
  // Store some details into data members so that further event
  // handling cat pick them from here.

  if (e.fIsMouse)
  {
    if (e.fEvent == FL_DRAG)
    {
      mMDragDX = e.fX - mMouseX;
      mMDragDY = e.fY - mMouseY;
    }
    else
    {
      mMDragDX = 0;
      mMDragDY = 0;
    }

    mMouseX = e.fX;
    mMouseY = e.fY;

    if (e.fEvent == FL_PUSH && e.fButton < 4)
    {
      mMPushX[e.fButton] = mMouseX;
      mMPushY[e.fButton] = mMouseY;
    }

    float yext = TMath::Tan(0.5*TMath::DegToRad()*mZFov)*mNearClip;
    float xext = yext*mWidth/mHeight;
    float xcam = xext*(2.0f*mMouseX/mWidth  - 1.0f);
    float ycam = yext*(2.0f*mMouseY/mHeight - 1.0f);

    mMouseRayPos.Zero();
    mCamFixTrans->MultiplyIP(mMouseRayPos);

    mMouseRayDir.Set(mNearClip, -xcam, -ycam);
    mMouseRayDir.Normalize();
    mCamFixTrans->RotateIP(mMouseRayDir);
  }
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

