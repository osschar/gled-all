// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_PupilInfo_H
#define GledCore_PupilInfo_H

#include <Glasses/ZList.h>
#include <Glasses/ZNode.h>

#include <Stones/ZColor.h>

#include <GL/gl.h>

class PupilInfo : public ZList {
  // 7777 RnrCtrl(1)
  MAC_RNR_FRIENDS(PupilInfo);

protected:
  void		_init();

  ZNode*	mCameraBase;	// X{gE} L{}

  ZNode*	mLookAt;	// X{gE} L{}
  Float_t	mLookAtMinDist; // X{gS} 7 Value(-range=>[0,1000,1,1000])

  ZNode*	mUpReference;	// X{gE} L{}
  UChar_t	mUpRefAxis;	// X{gS} 7 Value(-range=>[1,3,1,1], -join=>1)
  Bool_t	bUpRefLockDir;	// X{gS} 7 Bool()
  Float_t	mUpRefMinAngle;	// X{gS} 7 Value(-range=>[0,40,1,100])

  Int_t		mMaxDepth;	// X{gS} 7 Value(-range=>[1,1000,1,1])

  Int_t		mWidth;         // X{gE} 7 Value(-range=>[0,4096,1], -join=>1)
  Int_t		mHeight;        // X{gE} 7 Value(-range=>[0,4096,1])

  ZColor	mClearColor;	// X{PGS} 7 ColorButt(-join=>1)
  Float_t	mFOV;		// X{gS} 7 Value(-range=>[1,180,1,1])
  Float_t	mNearClip;	// X{gS} 7 Value(-width=>5, -range=>[0,1000,1,1000],-join=>1)
  Float_t	mFarClip;	// X{gS} 7 Value(-width=>5, -range=>[0,1000,1,1000])

  Int_t		mFrontMode;	// X{gS} 7 PhonyEnum(-vals=>[GL_POINT,Point, GL_LINE,Line, GL_FILL,Fill],-width=>6,-join=>1)
  Int_t		mBackMode;	// X{gS} 7 PhonyEnum(-vals=>[GL_POINT,Point, GL_LINE,Line, GL_FILL,Fill],-width=>6)
  Bool_t	bLiMo2Side;	// X{gS} 7 Bool(-join=>1)
  Bool_t	bBlend;		// X{gS} 7 Bool()

  Float_t	mMSRotFac;	// X{gS} 7 Value(-range=>[-1000,1000,1,100])
  Float_t	mMSMoveFac;	// X{gS} 7 Value(-range=>[-1000,1000,1,100])
  Float_t	mMoveOM;	// X{gS} 7 Value(-range=>[-30,30,1,100])
  Float_t	mAccelExp;	// X{gS} 7 Value(-range=>[-10,10,1,1000])

  Float_t	mCHSize;	// X{gS} 7 Value(-range=>[0,1,1,100], -width=>4,-join=>1)
  Int_t		mTextSize;	// X{gS} 7 Value(-range=>[0,100,1,1])

  Bool_t	bShowRPS;	// X{gS} 7 Bool(-join=>1)
  Bool_t	bRnrNames;	// X{gS} 7 Bool(-join=>1)
  Bool_t	bRnrTiles;	// X{gS} 7 Bool()
  Float_t	mNameOffset;	// X{gS} 7 Value(-range=>[0,2,1,10000])
  ZColor	mTextCol;	// X{PGS} 7 ColorButt(-join=>1)
  ZColor	mTileCol;	// X{PGS} 7 ColorButt()

  UInt_t	mBuffSize;	// X{gS} 7 Value(-range=>[0,65535,1,1])
  Int_t		mPickR;		// X{gS} 7 Value(-range=>[0,100,1,1],-join=>1)
  UChar_t       mPickDisp;      // X{gS} 7 PhonyEnum(-vals=>[0,Default, 1,"Z-distance", 2,"Z-percentage"], -width=>9)

public:
  PupilInfo(Text_t* n="PupilInfo", Text_t* t=0) : ZList(n,t) { _init(); }

  void SetCameraBase(ZNode* camerabase);
  void SetLookAt(ZNode* lookat);
  void SetUpReference(ZNode* upreference);

  void EmitDumpImageRay(const Text_t* filename="screenshot.tga"); // X{E} 7 MButt()
  void EmitImmediateRedrawRay();                                  // X{E} 7 MButt()

  void SetWidth(Int_t w)  { mWidth = w; EmitResizeRay(); }
  void SetHeight(Int_t h) { mHeight = h; EmitResizeRay(); }
  void EmitResizeRay(); // X{E} 7 MButt()

  ZTrans* ToPupilFrame(ZNode* node);
  ZTrans* ToCameraFrame(ZNode* node);

  // virtuals

#include "PupilInfo.h7"
  ClassDef(PupilInfo, 1)
}; // endclass PupilInfo

GlassIODef(PupilInfo);

#endif
