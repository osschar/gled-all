// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlMaterial_H
#define GledCore_ZGlMaterial_H

#include <Glasses/ZGlStateBase.h>
#include <Stones/ZColor.h>

#include <GL/gl.h>

class ZGlMaterial : public ZGlStateBase {
  // 7777 RnrCtrl("true, true, RnrBits(0,4,0,0)")
  MAC_RNR_FRIENDS(ZGlMaterial);

private:
  void _init();

protected:
  // Material properties
  ZGlStateBase::GlStateOp_e
                mMatOp;     // X{GS}  7 PhonyEnum()
  Int_t		mFace;	    // X{GS}  7 PhonyEnum(-vals=>[GL_FRONT,Front, GL_BACK,Back, GL_FRONT_AND_BACK,FrontAndBack], -width=>10, -join=>1)
  Float_t	mShininess; // X{GS}  7 Value(-range=>[0,1024,1,100], -width=>4)
  ZColor	mAmbient;   // X{PGS} 7 ColorButt(-join=>1)
  ZColor	mDiffuse;   // X{PGS} 7 ColorButt()
  ZColor	mSpecular;  // X{PGS} 7 ColorButt(-join=>1)
  ZColor	mEmission;  // X{PGS} 7 ColorButt()

  // Material mode
  ZGlStateBase::GlStateOp_e
                mModeOp;    // X{GS}  7 PhonyEnum()
  Int_t         mModeFace;  // X{GS}  7 PhonyEnum(-vals=>[GL_FRONT,Front, GL_BACK,Back, GL_FRONT_AND_BACK,FrontAndBack], -width=>10, -join=>1)
  Int_t		mModeColor; // X{GS}  7 PhonyEnum(-vals=>[GL_AMBIENT,Ambient, GL_DIFFUSE,Diffuse, GL_SPECULAR,Specular, GL_AMBIENT_AND_DIFFUSE,AmbAndDiff, GL_EMISSION,Emission], -width=>10)
public:
  ZGlMaterial(const Text_t* n="ZGlMaterial", const Text_t* t=0) : ZGlStateBase(n,t) { _init(); }


#include "ZGlMaterial.h7"
  ClassDef(ZGlMaterial, 1)
}; // endclass ZGlMaterial

GlassIODef(ZGlMaterial);

#endif
