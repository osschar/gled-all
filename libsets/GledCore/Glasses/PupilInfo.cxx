// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
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

#include <GL/glew.h>

ClassImp(PupilInfo);

Bool_t PupilInfo::sStereoDefault = false;

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
  mZFov     = 90;   mZSize    = 20;
  mYFac     = 1;    mXDist    = 10;
  mNearClip = 0.01; mFarClip  = 100;
  mZoomFac  = 1.1; // if fac = 1, linear zooming is used (see Pupil::handle())
  mDefZFov  = 90;   mDefZSize = 20;
  mMinZFov  = 0;
  mMaxZFov  = 160;

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

  bAllowHandlerSwitchInPupil = true;

  bStereo               = sStereoDefault;
  mStereoZeroParallax   = 0;
  mStereoEyeOffsetFac   = 1;
  mStereoFrustumAsymFac = 1;

  mRnrCamFix = 0;
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
  mZFov     = cam_info->GetZFov();
  mZSize    = cam_info->GetZSize();
  mYFac     = cam_info->GetYFac();
  mXDist    = cam_info->GetXDist();
  mNearClip = cam_info->GetNearClip();
  mFarClip  = cam_info->GetFarClip();
  mDefZFov  = cam_info->GetDefZFov();
  mDefZSize = cam_info->GetDefZSize();

  Stamp(FID());

  if(cam_info->GetFixCameraBase())
    EmitCameraHomeRay();
}

/**************************************************************************/

void PupilInfo::SmoothCameraHome(ZNode* new_base)
{
  if(new_base) {
    SetCameraBase(new_base);
    EmitRedrawRay(); // This forces redraw, reinitializes pupil's data.
  }
  EmitSmoothCameraHomeRay();
}

/**************************************************************************/

void PupilInfo::Zoom(Float_t delta)
{
  switch(mProjMode) {
  case P_Perspective: {
    Float_t fov = mZFov + delta;
    if (fov < mMinZFov) fov = mMinZFov;
    if (fov > mMaxZFov) fov = mMaxZFov;
    SetZFov (fov);
    SetZSize(2*mXDist*TMath::Tan(0.5*TMath::DegToRad()*fov));
    break;
  }
  case P_Orthographic: {
    SetZSize(mZSize + delta);
    SetZFov (2*TMath::RadToDeg()*TMath::ATan2(mZSize, mXDist));
  }
  }
}

void PupilInfo::ZoomFac(Float_t fac)
{
  switch(mProjMode) {
  case P_Perspective: {
    Float_t fov = fac*mZFov;
    if (fov < mMinZFov) fov = mMinZFov;
    if (fov > mMaxZFov) fov = mMaxZFov;
    SetZFov (fov);
    SetZSize(2*mXDist*TMath::Tan(0.5*TMath::DegToRad()*mZFov));
    break;
  }
  case P_Orthographic: {
    SetZSize(fac*mZSize);
    SetZFov (2*TMath::RadToDeg()*TMath::ATan2(mZSize, mXDist));
  }
  }
}

void PupilInfo::Home(Bool_t smooth)
{
  switch(mProjMode)
  {
    case P_Perspective:
    {
      if (mDefZFov && mDefZFov != mZFov)
      {
	SetZFov (mDefZFov);
	SetZSize(2*mXDist*TMath::Tan(0.5*TMath::DegToRad()*mZFov));
      }
      break;
    }
    case P_Orthographic:
    {
      SetZSize(mDefZSize);
      SetZFov (2*TMath::RadToDeg()*TMath::ATan2(mZSize, mXDist));
      break;
    }
  }
  if (smooth)
    EmitSmoothCameraHomeRay();
  else
    EmitCameraHomeRay();
}

void PupilInfo::SetupZFov(Float_t zfov)
{
  SetZFov(zfov);
  SetDefZFov(zfov);
}

void PupilInfo::SetupZSize(Float_t zsize)
{
  SetZSize(zsize);
  SetDefZSize(zsize);
}

/**************************************************************************/

ZTrans* PupilInfo::ToPupilFrame(ZNode* node)
{
  // Returns transformation from node->pupil reference frame.
  //
  // In general, it is a bad idea to call this function under a lens lock.

  if (node == 0) return 0;

  ZTrans *ret;
  {
    GLensReadHolder rd_lck(node);
    ret = new ZTrans(node->RefTrans());
  }

  set<ZGlass*> top_levels;
  {
    GMutexHolder lst_lck(mListMutex);
    Stepper<ZNode> s(this);
    while (s.step())
    {
      if (*s == node)
	return ret;
      top_levels.insert(*s);
    }
  }

  node = node->GetParent();
  while (node)
  {
    {
      GLensReadHolder rd_lck(node);
      ret->MultLeft(node->RefTrans());
    }

    if (top_levels.find(node) != top_levels.end())
      return ret;

    node = node->GetParent();
  }

  delete ret;
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

Bool_t PupilInfo::TransformMouseRayVectors(ZNode* ref, ZPoint& pos, ZPoint& dir)
{
  // Transforms last mouse-ray vectors into the frame of ref.
  // This multiples transformation matrices from camera-base upwards
  // until ref is reached -- the transformation of ref is not applied.

  pos = mMouseRayPos;
  dir = mMouseRayDir;

  // CamFix rotation is done in GLRnrDriver.
  // mRnrCamFix->MultiplyIP(pos);
  // mRnrCamFix->RotateIP  (dir);

  ZNode *node = *mCameraBase;
  while (node != ref)
  {
    if (!node)
      return false;

    GLensReadHolder rd_lck(node);

    node->ref_trans().MultiplyIP(pos);
    node->ref_trans().RotateIP  (dir);

    node = node->GetParent();
  }
  return true;
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

void PupilInfo::EmitRedrawRay(Bool_t signal_p)
{
  if (mQueen && mSaturn->AcceptsRays())
  {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_redraw, mTimeStamp, FID()));

    ray->CustomBuffer() << signal_p;
    mQueen->EmitRay(ray);
  }
}

void PupilInfo::EmitDumpImageRay(const TString& filename, Int_t n_tiles,
                                 Bool_t copy_p, Bool_t signal_p)
{
  if (mQueen && mSaturn->AcceptsRays())
  {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_dump_image, mTimeStamp, FID()));

    ray->CustomBuffer() << filename << n_tiles << copy_p << signal_p;
    mQueen->EmitRay(ray);
  }
}

void PupilInfo::Redraw()
{
  EmitRedrawRay();
}

void PupilInfo::RedrawWaitSignal()
{
  // This object must NOT be locked or rendering will deadlock.

  mDirectDumpCond.Lock();
  EmitRedrawRay(true);
  mDirectDumpCond.Wait();
  mDirectDumpCond.Unlock();
}

void PupilInfo::DumpImage(const TString& filename, Int_t n_tiles, Bool_t copy_p)
{
  EmitDumpImageRay(filename, n_tiles, copy_p, false);
}

void PupilInfo::DumpImageWaitSignal(const TString& filename, Int_t n_tiles,
                                    Bool_t copy_p)
{
  // This object must NOT be locked or rendering will deadlock.

  mDirectDumpCond.Lock();
  EmitDumpImageRay(filename, n_tiles, copy_p, true);
  mDirectDumpCond.Wait();
  mDirectDumpCond.Unlock();
}

void PupilInfo::ReceiveDumpFinishedSignal()
{
  // This is called from Pupil when rendering is finished.

  mDirectDumpCond.Lock();
  mDirectDumpCond.Signal();
  mDirectDumpCond.Unlock();
}
