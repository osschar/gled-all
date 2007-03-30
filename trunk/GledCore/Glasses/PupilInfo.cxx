// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// PupilInfo
//
// Configuration data for GL viewer.
//
// Camera orientation: x-forward, y-left, z-up. This should help understand
// viewing angle/size specification variables: mZFov, mYFac, mZSize, mXDist.
// ZFov & YFac used in perspective mode, mZSise & mYFac in orthographic mode.
// mXDist is used as a 'typical distance of objects' when changing from one
// mode to the other.
//
// Emits custom Rays: dump_image, resize_window, camera_home
//
// *Overlay*
//
// Two links control display and interaction with the overlay:
// _mOverlay_ is rendered after the rendering of the scene
// _mEventHandler_ if non-zero, fltk-like-events are sent to its renderer

#include "PupilInfo.h"
#include "PupilInfo.c7"
#include <Glasses/ZQueen.h>

#include <TMath.h>

ClassImp(PupilInfo)

void PupilInfo::_init()
{
  // Override from SubShellInfo:
  mCtorLibset = "GledCore";
  mCtorName   = "Pupil";

  // Basic config.
  mMaxRnrDepth = 100;
  bAutoRedraw  = true;
  mWidth = 640; mHeight = 480;
  mClearColor.rgba(0,0,0);

  // CameraInfo.
  mCameraBase    = 0;

  mLookAt        = 0;
  mLookAtMinDist = 0.1;

  mUpReference   = 0;
  mUpRefAxis     = 3;
  bUpRefLockDir  = true;
  mUpRefMinAngle = 10;

  mProjMode = P_Perspective;
  mZFov     = 90;   mZSize   = 20;
  mYFac     = 1;    mXDist   = 10;
  mNearClip = 0.01; mFarClip = 100;
  bZoomByFac = true;

  // Basic rendering options.
  mFrontMode = GL_FILL; mBackMode = GL_LINE;
  bLiMo2Side = false;
  bBlend     = false;

  // User interaction and feedback.
  mMSRotFac = 1; mMSMoveFac = 2; mMoveOM = -2; mAccelExp = 0.5;

  mCHSize       = 0.03; // Cross-hair size.
  mMPSize       = 0;    // Internal mouse-pointer size.
  mHomeAnimTime = 2;    // Time of smooth-home animation.

  bShowRPS = true; bShowView = true; bRnrNames = false;
  bRnrFakeOverlayInCapture = false; // Does not work in multi-tile capture.

  mBuffSize = 4096; mPickR = 5; mPickDisp = 0; mPickMaxN = 24;

  mPopupDx = 200; mPopupDy = 0;
  mPopupFx = 0;   mPopupFy = -0.5;

  mOverlay      = 0;
  mEventHandler = 0;
}

/**************************************************************************/

void PupilInfo::SetCameraBase(ZNode* camerabase)
{
  static const Exc_t _eh("PupilInfo::SetCameraBase ");

  if(camerabase != 0) {
    ZTrans* t = ToPupilFrame(camerabase);
    if(t == 0)
      throw(_eh + "camera not connected into pupil contents.");
    delete t;
  }
  set_link_or_die(mCameraBase.ref_link(), camerabase, FID());
}

void PupilInfo::SetLookAt(ZNode* lookat)
{
  static const Exc_t _eh("PupilInfo::SetLookAt ");

  if(lookat != 0) {
    ZTrans* t = ToPupilFrame(lookat);
    if(t == 0)
      throw(_eh + "camera not connected into pupil contents.");
    delete t;
  }
  set_link_or_die(mLookAt.ref_link(), lookat, FID());
}

void PupilInfo::SetUpReference(ZNode* upreference)
{
  static const Exc_t _eh("PupilInfo::SetUpReference ");

  if(upreference != 0) {
    ZTrans* t = ToPupilFrame(upreference);
    if(t == 0)
      throw("camera not connected into pupil contents.");
    delete t;
  }

  set_link_or_die(mUpReference.ref_link(), upreference, FID());
}

/**************************************************************************/

void PupilInfo::ImportCameraInfo(CameraInfo* cam_info)
{
  // Imports camera info data.

  // cat cbase.defs | perl -e 'undef $/; $l=<STDIN>; @x=split(/\s*=[^;]+\s*;\s*/, $l); for $d (@x) { $m=$d; $m=~s/^m|b//; print "  $d = cam_info->Get$m();\n";}'
  // + hand fixes for links/enums.

  static const Exc_t _eh("PupilInfo::ImportCameraInfo ");

  if(cam_info == 0) throw(_eh + "called with cam_info=0.");

  if(cam_info->GetFixCameraBase()) {
    SetCameraBase(cam_info->GetCameraBase());
  }
  if(cam_info->GetFixLookAt()) {
    SetLookAt(cam_info->GetLookAt());
    mLookAtMinDist = cam_info->GetLookAtMinDist();
  }
  if(cam_info->GetFixUpReference()) {
    SetUpReference(cam_info->GetUpReference());
    mUpRefAxis = cam_info->GetUpRefAxis();
    bUpRefLockDir = cam_info->GetUpRefLockDir();
    mUpRefMinAngle = cam_info->GetUpRefMinAngle();
  }
  mProjMode = (Projection_e) cam_info->GetProjMode();
  mZFov = cam_info->GetZFov();
  mZSize = cam_info->GetZSize();
  mYFac = cam_info->GetYFac();
  mXDist = cam_info->GetXDist();
  mNearClip = cam_info->GetNearClip();
  mFarClip = cam_info->GetFarClip();

  Stamp(FID());

  if(cam_info->GetFixCameraBase())
    EmitCameraHomeRay();
}

/**************************************************************************/

void PupilInfo::SmoothCameraHome(ZNode* new_base)
{
  if(new_base) {
    SetCameraBase(new_base);
    EmitDumpImageRay(); // This forces redraw, reinitializes pupil's data.
  }
  EmitSmoothCameraHomeRay();
}

/**************************************************************************/

void PupilInfo::Zoom(Float_t delta)
{ 
  switch(mProjMode) {
  case P_Perspective: {
    SetZFov (mZFov + 5*delta);
    SetZSize(2*mXDist*TMath::Tan(TMath::DegToRad()*mZFov/2));
    break;
  }
  case P_Orthographic: {
    SetZSize(mZSize += 0.5*delta);
    SetZFov (2*TMath::RadToDeg()*TMath::ATan2(mZSize, mXDist));
  }
  }
}

void PupilInfo::ZoomFac(Float_t fac)
{ 
  switch(mProjMode) {
  case P_Perspective: {
    SetZFov (fac*mZFov);
    SetZSize(2*mXDist*TMath::Tan(TMath::DegToRad()*mZFov/2));
    break;
  }
  case P_Orthographic: {
    SetZSize(fac*mZSize);
    SetZFov (2*TMath::RadToDeg()*TMath::ATan2(mZSize, mXDist));
  }
  }
}

/**************************************************************************/

ZTrans* PupilInfo::ToPupilFrame(ZNode* node)
{
  // Returns transformation from node->pupil reference frame.

  // This should use parental-list, like similar stuff in ZNode.

  if(node == 0) return 0;

  GMutexHolder lst_lck(mListMutex);
  Stepper<ZNode> s(this);
  while(s.step()) {
    if(*s == node) {
      return new ZTrans(s->RefTrans());
    } else {
      ZTrans* t = node->ToNode(*s);
      if(t != 0) {
	ZTrans* ret = new ZTrans(s->RefTrans());
	*ret *= *t;
	delete t;
	return ret;
      }
    }
  }
  return 0;
}

ZTrans* PupilInfo::ToCameraFrame(ZNode* node)
{
  // Returns transformation from node->camera reference frame.
  // Camera's matrix is not multiplied in the result (as it is not known).

  ZTrans* n2p = ToPupilFrame(node);
  if(n2p == 0) return 0;
  
  ZNode* cam_base = mCameraBase.get();
  if(cam_base) {
    ZTrans* c2p = ToPupilFrame(cam_base);
    if(c2p == 0) {
      delete n2p;
      return 0;
    }
    c2p->Invert();
    *c2p *= *n2p;
    delete n2p;
    return c2p;
  }
  return n2p;
}

/**************************************************************************/

void PupilInfo::EmitResizeRay()
{
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_resize_window, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}

void PupilInfo::EmitCameraHomeRay()
{
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_camera_home, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}

void PupilInfo::EmitSmoothCameraHomeRay()
{
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_smooth_camera_home, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}

/**************************************************************************/

void PupilInfo::EmitDumpImageRay(TString filename, Int_t n_tiles, Bool_t signal_p)
{
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_dump_image, mTimeStamp, FID()));

    TBufferFile cbuff(TBuffer::kWrite);
    cbuff << filename;
    cbuff << n_tiles;
    cbuff << signal_p;
    ray->SetCustomBuffer(cbuff);

    mQueen->EmitRay(ray);
  }
}

void PupilInfo::DumpImage(TString filename, Int_t n_tiles)
{
  EmitDumpImageRay(filename, n_tiles, false);
}

void PupilInfo::ImmediateRedraw()
{
  EmitDumpImageRay();
}

void PupilInfo::DumpImageWaitSignal(TString filename, Int_t n_tiles)
{
  // *this must NOT be locked or rendering will deadlock!

  mDirectDumpCond.Lock();
  EmitDumpImageRay(filename, n_tiles, true);
  mDirectDumpCond.Wait();
  mDirectDumpCond.Unlock();
}

void PupilInfo::ReceiveDumpFinishedSignal()
{
  mDirectDumpCond.Lock();
  mDirectDumpCond.Signal();
  mDirectDumpCond.Unlock();
}
