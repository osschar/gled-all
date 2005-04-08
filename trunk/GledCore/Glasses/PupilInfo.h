// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_PupilInfo_H
#define GledCore_PupilInfo_H

#include <Glasses/SubShellInfo.h>
#include <Glasses/CameraInfo.h>
#include <Glasses/ZNode.h>
#include <Net/Ray.h>

#include <Stones/ZColor.h>

#include <GL/gl.h>


class PupilInfo : public SubShellInfo {
  MAC_RNR_FRIENDS(PupilInfo);
  friend class Pupil;

public:
  enum Projection_e { P_Perspective, P_Orthographic };
  enum PrivRayQN_e  { PRQN_offset = RayNS::RQN_user_0,
		      PRQN_dump_image,
		      PRQN_resize_window,
		      PRQN_camera_home
  };

private:
  void		_init();

protected:
  // Basic config.
  Int_t		mMaxRnrDepth;	// X{gS} 7 Value(-range=>[1,1000,1,1])
  Int_t		mWidth;         // X{gS} Ray{Resize} 7 Value(-range=>[0,4096,1], -join=>1)
  Int_t		mHeight;        // X{gS} Ray{Resize} 7 Value(-range=>[0,4096,1])
  ZColor	mClearColor;	// X{PRGS} 7 ColorButt()


  // CameraInfo.
  ZNode*	mCameraBase;	// X{gE} L{}

  ZNode*	mLookAt;	// X{gE} L{}
  Float_t	mLookAtMinDist; // X{gS} 7 Value(-range=>[0,1000,1,1000])

  ZNode*	mUpReference;	// X{gE} L{}
  UChar_t	mUpRefAxis;	// X{gS} 7 Value(-range=>[1,3,1,1], -join=>1)
  Bool_t	bUpRefLockDir;	// X{gS} 7 Bool()
  Float_t	mUpRefMinAngle;	// X{gS} 7 Value(-range=>[0,90,1,100])

  Projection_e	mProjMode;	// X{GS} 7 PhonyEnum()
  Float_t	mZFov;		// X{gS} 7 Value(-width=>6, -range=>[1,180,1,100], -join=>1)
  Float_t	mZSize;		// X{gS} 7 Value(-width=>6, -range=>[1e-3,1000, 1,1000])
  Float_t	mYFac;		// X{gS} 7 Value(-width=>6, -range=>[1e-3,1000, 1,1000], -join=>1)
  Float_t	mXDist;		// X{gS} 7 Value(-width=>6, -range=>[1e-3,1000, 1,1000])
  Float_t	mNearClip;	// X{gS} 7 Value(-width=>6, -range=>[0,1000,1,1000],-join=>1)
  Float_t	mFarClip;	// X{gS} 7 Value(-width=>6, -range=>[0,1000,1,1000])

  // Basic rendering options.
  Int_t		mFrontMode;	// X{gS} 7 PhonyEnum(-vals=>[GL_POINT,Point, GL_LINE,Line, GL_FILL,Fill],-width=>6,-join=>1)
  Int_t		mBackMode;	// X{gS} 7 PhonyEnum(-vals=>[GL_POINT,Point, GL_LINE,Line, GL_FILL,Fill],-width=>6)
  Bool_t	bLiMo2Side;	// X{gS} 7 Bool(-join=>1)
  Bool_t	bBlend;		// X{gS} 7 Bool()

  // User interaction and feedback.
  Float_t	mMSRotFac;	// X{gS} 7 Value(-range=>[-1000,1000,1,100])
  Float_t	mMSMoveFac;	// X{gS} 7 Value(-range=>[-1000,1000,1,100])
  Float_t	mMoveOM;	// X{gS} 7 Value(-range=>[-30,30,1,100])
  Float_t	mAccelExp;	// X{gS} 7 Value(-range=>[-10,10,1,1000])

  Float_t	mCHSize;	// X{gS} 7 Value(-range=>[0,1,1,100], -width=>4)

  Bool_t	bShowRPS;	// X{gS} 7 Bool(-join=>1)
  Bool_t	bRnrNames;	// X{gS} 7 Bool()

  UInt_t	mBuffSize;	// X{gS} 7 Value(-range=>[0,65535,1,1])
  Int_t		mPickR;		// X{gS} 7 Value(-range=>[0,100,1,1],-join=>1)
  UChar_t       mPickDisp;      // X{gS} 7 PhonyEnum(-vals=>[0,Default, 1,"Z-distance", 2,"Z-percentage"], -width=>9)

  Int_t		mPopupDx;	// X{GS} 7 Value(-range=>[-2000,2000,1,1], -join=>1)
  Int_t		mPopupDy;	// X{GS} 7 Value(-range=>[-2000,2000,1,1])
  Float_t	mPopupFx;	// X{GS} 7 Value(-range=>[-10,10,1,10], -join=>1)
  Float_t	mPopupFy;	// X{GS} 7 Value(-range=>[-10,10,1,10])

public:
  PupilInfo(const Text_t* n="PupilInfo", const Text_t* t=0) :
    SubShellInfo(n,t) { _init(); }

  void SetCameraBase(ZNode* camerabase);
  void SetLookAt(ZNode* lookat);
  void SetUpReference(ZNode* upreference);

  void ImportCameraInfo(CameraInfo* cam_info); // X{E} C{1} 7 MCWButt()

  void Zoom(Float_t delta); // X{E}

  void EmitDumpImageRay(const Text_t* filename="screenshot.tga"); // X{E} 7 MButt()
  void EmitImmediateRedrawRay();                                  // X{E} 7 MButt()

  void EmitResizeRay(); // X{E} 7 MButt()

  void EmitCameraHomeRay(); // X{E} 7 MButt()

  ZTrans* ToPupilFrame(ZNode* node);
  ZTrans* ToCameraFrame(ZNode* node);

  // virtuals

#include "PupilInfo.h7"
  ClassDef(PupilInfo, 1)
}; // endclass PupilInfo

GlassIODef(PupilInfo);

#endif
