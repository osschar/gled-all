// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlLightModel_H
#define GledCore_ZGlLightModel_H

#include <Glasses/ZRnrModBase.h>
#include <Stones/ZColor.h>

#include <GL/gl.h>

class ZGlLightModel : public ZRnrModBase {
  // 7777 RnrCtrl(RnrBits(0,4,0,0))
  MAC_RNR_FRIENDS(ZGlLightModel);

private:
  void _init();

protected:
  // Light model
  ZRnrModBase::Operation_e
                mLightModelOp;    // X{GS}  7 PhonyEnum()
  ZColor	mLiMoAmbient;     // X{PGS} 7 ColorButt(-join=>1)
  Int_t		mLiMoColorCtrl;	  // X{GS}  7 PhonyEnum(-width=>10, -vals=>[GL_SINGLE_COLOR,Single, GL_SEPARATE_SPECULAR_COLOR,SeparateSpecular]);
  Bool_t	bLiMoLocViewer;	  // X{GS}  7 Bool(-join=>1)
  Bool_t	bLiMoTwoSide;	  // X{GS}  7 Bool()

  // Shade model
  ZRnrModBase::Operation_e
                mShadeModelOp;    // X{GS}  7 PhonyEnum()
  Int_t		mShadeModel;      // X{GS}  7 PhonyEnum(-vals=>[GL_SMOOTH,Smooth, GL_FLAT,Flat], join=>1)
  Int_t		mFrontFace;       // X{GS}  7 PhonyEnum(-vals=>[GL_CCW,CCW, GL_CW,CW])
  Int_t		mFrontMode;	  // X{GS}  7 PhonyEnum(-vals=>[GL_POINT,Point, GL_LINE,Line, GL_FILL,Fill], -width=>8, -join=>1)
  Int_t		mBackMode;        // X{GS}  7 PhonyEnum(-vals=>[GL_POINT,Point, GL_LINE,Line, GL_FILL,Fill], -width=>8)

  // Face culling
  ZRnrModBase::Operation_e
                mFaceCullOp;       // X{GS}  7 PhonyEnum()
  Int_t		mFaceCullMode;     // X{GS}  7 PhonyEnum(-vals=>[GL_FRONT,Front, GL_BACK,Back, GL_FRONT_AND_BACK,FrontAndBack])

public:
  ZGlLightModel(const Text_t* n="ZGlLightModel", const Text_t* t=0) : ZRnrModBase(n,t) { _init(); }


#include "ZGlLightModel.h7"
  ClassDef(ZGlLightModel, 1) // Control of GL light & shade model, polygon mode and face culling
}; // endclass ZGlLightModel

GlassIODef(ZGlLightModel);

#endif
