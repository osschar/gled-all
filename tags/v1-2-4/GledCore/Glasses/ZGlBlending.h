// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlBlending_H
#define GledCore_ZGlBlending_H

#include <Glasses/ZGlStateBase.h>
#include <Stones/ZColor.h>

#include <GL/gl.h>

class ZGlBlending : public ZGlStateBase {
  // 7777 RnrCtrl("true, true, RnrBits(0,4,0,0)")
  MAC_RNR_FRIENDS(ZGlBlending);

private:
  void _init();

protected:
  // Blend mode
  ZGlStateBase::GlStateOp_e
                mBlendOp;     // X{GS}  7 PhonyEnum()
  Int_t		mBSrcFac;     // X{GS}  7 PhonyEnum(-width=>12, -vals=>[GL_ZERO,Zero, GL_ONE,One, GL_DST_COLOR,DstColor, GL_ONE_MINUS_DST_COLOR,OMDstColor, GL_SRC_ALPHA,SrcAlpha, GL_ONE_MINUS_SRC_ALPHA,OMSrcAlpha, GL_DST_ALPHA,DstAplha, GL_ONE_MINUS_DST_ALPHA,OMDstAlpha, GL_SRC_ALPHA_SATURATE,SrcAlphaSaturate, GL_CONSTANT_COLOR,ConstColor, GL_ONE_MINUS_CONSTANT_COLOR,OMConstCol, GL_CONSTANT_ALPHA,ConstAlpha, GL_ONE_MINUS_CONSTANT_ALPHA,OMConstAlpha], -join=>1)
  Int_t		mBDstFac;     // X{GS}  7 PhonyEnum(-width=>10, -vals=>[GL_ZERO,Zero, GL_ONE,One, GL_SRC_COLOR,SrcColor, GL_ONE_MINUS_SRC_COLOR,OMSrcColor, GL_SRC_ALPHA,SrcAlpha, GL_ONE_MINUS_SRC_ALPHA,OMSrcAlpha, GL_DST_ALPHA,DstAplha, GL_ONE_MINUS_DST_ALPHA,OMDstAlpha, GL_CONSTANT_COLOR,ConstColor, GL_ONE_MINUS_CONSTANT_COLOR,OMConstCol, GL_CONSTANT_ALPHA,ConstAlpha, GL_ONE_MINUS_CONSTANT_ALPHA,OMConstAlpha])

  // Antialiasing points, lines
  ZGlStateBase::GlStateOp_e
                mAntiAliasOp; // X{GS}  7 PhonyEnum()
  Bool_t	bPointSmooth; // X{GS}  7 Bool()
  Float_t	mPointSize;   // X{GS}  7 Value(-width=>6, -range=>[0.01,100,1,100], join=>1)
  Int_t		mPointHint;   // X{GS}  7 PhonyEnum(-width=>6, -vals=>[GL_NICEST,Nicest, GL_FASTEST,Fastest, GL_DONT_CARE,DontCare])
  Bool_t	bLineSmooth;  // X{GS}  7 Bool()
  Float_t	mLineWidth;   // X{GS}  7 Value(-width=>6, -range=>[0.01,10,1,100], join=>1)
  Int_t		mLineHint;    // X{GS}  7 PhonyEnum(-width=>6, -vals=>[GL_NICEST,Nicest, GL_FASTEST,Fastest, GL_DONT_CARE,DontCare])

  // Fog
  ZGlStateBase::GlStateOp_e
                mFogOp;       // X{GS}  7 PhonyEnum()
  Int_t		mFogMode;     // X{GS}  7 PhonyEnum(-width=>6, -vals=>[GL_EXP,Exp, GL_EXP2,Exp2, GL_LINEAR,Linear], -join=>1)
  Int_t		mFogHint;     // X{GS}  7 PhonyEnum(-width=>6, -vals=>[GL_NICEST,Nicest, GL_FASTEST,Fastest, GL_DONT_CARE,DontCare])
  ZColor	mFogColor;    // X{PGS} 7 ColorButt(-join=>1)
  Float_t	mFogDensity;  // X{GS}  7 Value(-range=>[0,1000,1,1000])
  Float_t	mFogBeg;      // X{GS}  7 Value(-range=>[-1000,1000,1,100], -join=>1)
  Float_t	mFogEnd;      // X{GS}  7 Value(-range=>[-1000,1000,1,100])

public:
  ZGlBlending(const Text_t* n="ZGlBlending", const Text_t* t=0) : ZGlStateBase(n,t) { _init(); }


#include "ZGlBlending.h7"
  ClassDef(ZGlBlending, 1)
}; // endclass ZGlBlending

GlassIODef(ZGlBlending);

#endif
