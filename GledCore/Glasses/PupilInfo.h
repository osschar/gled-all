// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_PupilInfo_H
#define Gled_PupilInfo_H

#include <Glasses/ZList.h>
#include <Glasses/ZNode.h>

#include <Stones/ZColor.h>

#include <GL/gl.h>

class PupilInfo : public ZList {
  // 7777 RnrCtrl("true, 7, RnrBits(2,4,6,0, 0,0,0,5)")
  MAC_RNR_FRIENDS(PupilInfo);

protected:
  void		_init();

  ZNode*	mCameraBase;	// X{GE} L{}
  ZNode*	mLookAt;	// X{GE} L{}
  ZNode*	mUpReference;	// X{GE} L{}
  UChar_t	mUpRefAxis;	// X{GS} 7 Value(-range=>[1,3,1,1])

  Int_t		mMaxDepth;	// X{GS} 7 Value(-range=>[1,1000,1,1])

  ZColor	mClearColor;	// X{PGS} 7 ColorButt(-join=>1)
  Float_t	mFOV;		// X{GS} 7 Value(-range=>[1,180,1,1])
  Float_t	mNearClip;	// X{GS} 7 Value(-range=>[0,1000,1,100],-join=>1)
  Float_t	mFarClip;	// X{GS} 7 Value(-range=>[0,1000,1,100])

  Int_t		mFrontMode;	// X{GS} 7 PhonyEnum(-vals=>[GL_POINT,Point, GL_LINE,Line, GL_FILL,Fill],-width=>8,-join=>1)
  Int_t		mBackMode;	// X{GS} 7 PhonyEnum(-vals=>[GL_POINT,Point, GL_LINE,Line, GL_FILL,Fill],-width=>8)
  Bool_t	bLiMo2Side;	// X{GS} 7 Bool(-join=>1)
  Bool_t	bBlend;		// X{GS} 7 Bool()

  Float_t	mMSRotFac;	// X{GS} 7 Value(-range=>[-1000,1000,1,10],-join=>1)
  Float_t	mMSMoveFac;	// X{GS} 7 Value(-range=>[-1000,1000,1,10])

  Float_t	mCHSize;	// X{GS} 7 Value(-range=>[0,1,1,100])

  UInt_t	mBuffSize;	// X{GS} 7 Value(-range=>[0,65535,1,1])
  Int_t		mPickW;		// X{GS} 7 Value(-range=>[0,100,1,1],-join=>1)
  Int_t		mPickH;		// X{GS} 7 Value(-range=>[0,100,1,1])

public:
  PupilInfo(Text_t* n="PupilInfo", Text_t* t=0) : ZList(n,t) { _init(); }

  void SetCameraBase(ZNode* camerabase);
  void SetLookAt(ZNode* lookat);
  void SetUpReference(ZNode* upreference);

  ZTrans* ToPupilFrame(ZNode* node);

  // virtuals

#include "PupilInfo.h7"
  ClassDef(PupilInfo, 1)
}; // endclass PupilInfo

GlassIODef(PupilInfo);

#endif
