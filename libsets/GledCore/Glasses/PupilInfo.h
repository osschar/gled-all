// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_PupilInfo_H
#define GledCore_PupilInfo_H

#include <Glasses/SubShellInfo.h>
#include <Glasses/CameraInfo.h>
#include <Glasses/ZNode.h>
#include <Gled/GCondition.h>
#include <Eye/Ray.h>

#include <Stones/ZColor.h>

class PupilInfo : public SubShellInfo
{
  // 7777 AddViewInclude(GL/glew.h)
  MAC_RNR_FRIENDS(PupilInfo);
  friend class Pupil;

public:
  enum Projection_e { P_Perspective, P_Orthographic };
  enum PrivRayQN_e  { PRQN_offset = RayNS::RQN_user_0,
		      PRQN_resize_window,
		      PRQN_camera_home,
		      PRQN_smooth_camera_home,
		      PRQN_redraw,
		      PRQN_dump_image
  };

private:
  void		_init();

protected:
  // Basic config.
  Int_t		mMaxRnrDepth;	// X{gS} 7 Value(-range=>[1,1000,1,1], -join=>1)
  Bool_t        bAutoRedraw;    // X{GS} 7 Bool()
  Int_t		mWidth;         // X{gS} Ray{Resize} 7 Value(-range=>[0,4096,1], -join=>1)
  Int_t		mHeight;        // X{gS} Ray{Resize} 7 Value(-range=>[0,4096,1])
  ZColor	mClearColor;	// X{PRGS} 7 ColorButt()

  // CameraInfo.
  ZLink<ZNode>	mCameraBase;	// X{gE} L{}

  ZLink<ZNode>	mLookAt;	// X{gE} L{}
  Float_t	mLookAtMinDist; // X{gS} 7 Value(-range=>[0,1000,1,1000])

  ZLink<ZNode>	mUpReference;	// X{gE} L{}
  UChar_t	mUpRefAxis;	// X{gS} 7 Value(-range=>[1,3,1,1], -join=>1)
  Bool_t	bUpRefLockDir;	// X{gS} 7 Bool()
  Float_t	mUpRefMinAngle;	// X{gS} 7 Value(-range=>[0,90,1,100])

  Projection_e	mProjMode;	// X{GS} 7 PhonyEnum()
  Float_t	mZFov;		// X{gS} 7 Value(-width=>6, -range=>[1e-6,180,  1,1000], -join=>1)
  Float_t	mZSize;		// X{gS} 7 Value(-width=>6, -range=>[1e-6,1000, 1,1000])
  Float_t	mYFac;		// X{gS} 7 Value(-width=>6, -range=>[1e-6,1000, 1,1000], -join=>1)
  Float_t	mXDist;		// X{gS} 7 Value(-width=>6, -range=>[1e-6,1000, 1,1000])
  Float_t	mNearClip;	// X{gS} 7 Value(-width=>6, -range=>[0,100000,1,1000],-join=>1)
  Float_t	mFarClip;	// X{gS} 7 Value(-width=>6, -range=>[0,100000,1,1000])
  Float_t	mZoomFac;       // X{gS} 7 Value(-width=>6, -range=>[1,2,1,10000])
  Float_t   	mDefZFov;	// X{gS}
  Float_t   	mDefZSize;	// X{gS}
  Float_t       mMinZFov;	// X{gS}
  Float_t       mMaxZFov;	// X{gS}

  // Basic rendering options.
  Int_t		mFrontMode;	// X{gS} 7 PhonyEnum(-vals=>[GL_POINT,Point, GL_LINE,Line, GL_FILL,Fill],-width=>6,-join=>1)
  Int_t		mBackMode;	// X{gS} 7 PhonyEnum(-vals=>[GL_POINT,Point, GL_LINE,Line, GL_FILL,Fill],-width=>6)
  Bool_t	bLiMo2Side;	// X{gS} 7 Bool(-join=>1)
  Bool_t	bBlend;		// X{gS} 7 Bool()

  // User interaction and feedback.
  Float_t	mMSRotFac;	// X{gS} 7 Value(-range=>[-1000,1000,1,100], -join=>1)
  Float_t	mMSMoveFac;	// X{gS} 7 Value(-range=>[-1000,1000,1,100])
  Float_t	mMoveOM;	// X{gS} 7 Value(-range=>[-30,30,1,100], -join=>1)
  Float_t	mAccelExp;	// X{gS} 7 Value(-range=>[-10,10,1,1000])

  Float_t	mCHSize;	// X{GS} 7 Value(-range=>[0,1,  1,1000], -width=>4, -join=>1)
  Float_t       mMPSize;        // X{GS} 7 Value(-range=>[0,1,  1,1000], -width=>4)
  Float_t       mHomeAnimTime;  // X{GS} 7 Value(-range=>[0,10, 1,1000])

  Bool_t	bShowRPS;	// X{gS} 7 Bool(-join=>1)
  Bool_t	bShowView;	// X{gS} 7 Bool(-join=>1)
  Bool_t	bRnrNames;	// X{gS} 7 Bool()
  Bool_t	bRnrFakeOverlayInCapture; // X{GS} 7 Bool()

  UInt_t	mBuffSize;	// X{gS} 7 Value(-range=>[0,65535,1])
  Int_t		mPickR;		// X{gS} 7 Value(-range=>[0,100,1],-join=>1)
  UChar_t       mPickDisp;      // X{gS} 7 PhonyEnum(-vals=>[0,Default, 1,"Z-distance", 2,"Z-percentage"], -width=>9, -join=>1)
  Int_t         mPickMaxN;      // X{GS} 7 Value(-range=>[1,100,1])

  Int_t		mPopupDx;	// X{GS} 7 Value(-range=>[-2000,2000,1,1], -join=>1)
  Int_t		mPopupDy;	// X{GS} 7 Value(-range=>[-2000,2000,1,1])
  Float_t	mPopupFx;	// X{GS} 7 Value(-range=>[-10,10,1,10], -join=>1)
  Float_t	mPopupFy;	// X{GS} 7 Value(-range=>[-10,10,1,10])

  ZLink<ZGlass> mOverlay;       // X{GS} L{f}
  FID_t         mOverlay_fid;   //!

  ZLink<ZGlass> mEventHandler;     // X{GS} L{f}
  FID_t         mEventHandler_fid; //!

  Bool_t        bAllowHandlerSwitchInPupil; // X{GS} 7 Bool()

  Bool_t        bStereo;               // X{GS}
  Double_t      mStereoZeroParallax;   // X{GS} 7 Value(-range=>[-1,1, 1,1000])
  Double_t      mStereoEyeOffsetFac;   // X{GS} 7 Value(-range=>[ 0,2, 1,1000])
  Double_t      mStereoFrustumAsymFac; // X{GS} 7 Value(-range=>[ 0,2, 1,1000])

  // Locals where GL renderer stores actual parameters used for last render.
  // Protect with a lock?
  // Should this be passed in some struct when in event-handling?
  // Store last / current with a lock?
  ZTrans*  mRnrCamFix;   //! X{G}
  Int_t    mRnrScreenW;  //! X{G}
  Int_t    mRnrScreenH;  //! X{G}

  Int_t    mMouseX;      //! X{G}
  Int_t    mMouseY;      //! X{G}
  Int_t    mMouseDragDX; //! X{G}
  Int_t    mMouseDragDY; //! X{G}
  ZPoint   mMouseRayPos; //! X{R}
  ZPoint   mMouseRayDir; //! X{R}

  // Direct dump hack.
  GCondition mDirectDumpCond; //!
  // Could have max wait time.

public:
  PupilInfo(const Text_t* n="PupilInfo", const Text_t* t=0) :
    SubShellInfo(n,t) { _init(); }

  void SetCameraBase(ZNode* camerabase);
  void SetLookAt(ZNode* lookat);
  void SetUpReference(ZNode* upreference);

  void ImportCameraInfo(CameraInfo* cam_info); // X{E} C{1} 7 MCWButt()

  void SmoothCameraHome(ZNode* new_base=0);    // X{E} C{1} 7 MCWButt()

  void Zoom(Float_t delta);       // X{E}
  void ZoomFac(Float_t fac);      // X{E}
  void Home(Bool_t smooth=false); // X{E}

  void SetupZFov (Float_t zfov);
  void SetupZSize(Float_t zsize);

  ZTrans* ToPupilFrame(ZNode* node);
  ZTrans* ToCameraFrame(ZNode* node);

  Bool_t  TransformMouseRayVectors(ZNode* ref, ZPoint& pos, ZPoint& dir);

  void EmitResizeRay();           // X{E} 7 MButt(-join=>1)
  void EmitCameraHomeRay();       // X{E} 7 MButt()
  void EmitSmoothCameraHomeRay(); // X{E} 7 MButt()

  void EmitRedrawRay(Bool_t signal_p=false);
  void EmitDumpImageRay(const TString& filename, Int_t n_tiles=1,
                        Bool_t copy_p=false, Bool_t signal_p=false);

  void Redraw();                                  // X{E} 7 MButt(-join=>1)
  void RedrawWaitSignal();

  void DumpImage(const TString& filename="screenshot", Int_t n_tiles=1,
                 Bool_t copy_p=false);            // X{E} 7 MCWButt()
  void DumpImageWaitSignal(const TString& filename="screenshot", Int_t n_tiles=1,
                           Bool_t copy_p=false);

  void ReceiveDumpFinishedSignal();

  static Bool_t sStereoDefault;

#include "PupilInfo.h7"
  ClassDef(PupilInfo, 1);
}; // endclass PupilInfo


#endif
