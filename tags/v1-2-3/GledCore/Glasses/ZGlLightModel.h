// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlLightModel_H
#define GledCore_ZGlLightModel_H

#include <Glasses/ZGlStateBase.h>
#include <Stones/ZColor.h>

#include <GL/gl.h>

class ZGlLightModel : public ZGlStateBase {
  // 7777 RnrCtrl("false, 1, RnrBits(0,1,0,0)")
  MAC_RNR_FRIENDS(ZGlLightModel);

private:
  void _init();

protected:
  // Light model
  ZGlStateBase::GlStateOp_e
                mLightModelOp;     // X{GS}  7 PhonyEnum()
  ZColor	mLiMoAmbient;     // X{PGS} 7 ColorButt(-join=>1)
  Int_t		mLiMoColorCtrl;	  // X{GS}  7 PhonyEnum(-width=>10, -vals=>[GL_SINGLE_COLOR,Single, GL_SEPARATE_SPECULAR_COLOR,SeparateSpecular]);
  Bool_t	bLiMoLocViewer;	  // X{GS}  7 Bool(-join=>1)
  Bool_t	bLiMoTwoSide;	  // X{GS}  7 Bool()

  // Shade model
  ZGlStateBase::GlStateOp_e
                mShadeModelOp;    // X{GS}  7 PhonyEnum()
  Int_t		mShadeModel;      // X{GS}  7 PhonyEnum(-vals=>[GL_SMOOTH,Smooth, GL_FLAT,Flat], join=>1)
  Int_t		mFrontFace;       // X{GS}  7 PhonyEnum(-vals=>[GL_CCW,CCW, GL_CW,CW])
  Int_t		mFrontMode;	  // X{GS}  7 PhonyEnum(-vals=>[GL_POINT,Point, GL_LINE,Line, GL_FILL,Fill], -width=>8, -join=>1)
  Int_t		mBackMode;        // X{GS}  7 PhonyEnum(-vals=>[GL_POINT,Point, GL_LINE,Line, GL_FILL,Fill], -width=>8)

  // Face culling
  ZGlStateBase::GlStateOp_e
                mFaceCullOp;       // X{GS}  7 PhonyEnum()
  Int_t		mFaceCullMode;     // X{GS}  7 PhonyEnum(-vals=>[GL_FRONT,Front, GL_BACK,Back, GL_FRONT_AND_BACK,FrontAndBack])
public:
  ZGlLightModel(const Text_t* n="ZGlLightModel", const Text_t* t=0) : ZGlStateBase(n,t) { _init(); }


#include "ZGlLightModel.h7"
  ClassDef(ZGlLightModel, 1)
}; // endclass ZGlLightModel

GlassIODef(ZGlLightModel);

#endif
