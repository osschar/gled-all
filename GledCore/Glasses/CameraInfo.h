// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_CameraInfo_H
#define GledCore_CameraInfo_H

#include <Glasses/ZGlass.h>
#include <Glasses/ZNode.h>

class CameraInfo : public ZGlass {
  MAC_RNR_FRIENDS(CameraInfo);

public:
  enum Projection_e { P_Perspective, P_Orthographic };

private:
  void _init();

protected:
  Bool_t	bFixCameraBase;  // X{GS} 7 Bool()
  Bool_t	bFixLookAt;      // X{GS} 7 Bool()
  Bool_t	bFixUpReference; // X{GS} 7 Bool()

  ZNode*	mCameraBase;	// X{gS} L{}

  ZNode*	mLookAt;	// X{gS} L{}
  Float_t	mLookAtMinDist; // X{gS} 7 Value(-range=>[0,1000,1,1000])

  ZNode*	mUpReference;	// X{gS} L{}
  UChar_t	mUpRefAxis;	// X{gS} 7 Value(-range=>[1,3,1,1], -join=>1)
  Bool_t	bUpRefLockDir;	// X{gS} 7 Bool()
  Float_t	mUpRefMinAngle;	// X{gS} 7 Value(-range=>[0,40,1,100])

  Projection_e	mProjMode;	// X{GS} 7 PhonyEnum()
  Float_t	mZFov;		// X{gS} 7 Value(-width=>6, -range=>[1,180,1,100], -join=>1)
  Float_t	mZSize;		// X{gS} 7 Value(-width=>6, -range=>[1e-3,1000, 1,1000])
  Float_t	mYFac;		// X{gS} 7 Value(-width=>6, -range=>[1e-3,1000, 1,1000], -join=>1)
  Float_t	mXDist;		// X{gS} 7 Value(-width=>6, -range=>[1e-3,1000, 1,1000])
  Float_t	mNearClip;	// X{gS} 7 Value(-width=>6, -range=>[0,1000,1,1000],-join=>1)
  Float_t	mFarClip;	// X{gS} 7 Value(-width=>6, -range=>[0,1000,1,1000])

public:
  CameraInfo(const Text_t* n="CameraInfo", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }


#include "CameraInfo.h7"
  ClassDef(CameraInfo, 1)
}; // endclass CameraInfo

GlassIODef(CameraInfo);

#endif
