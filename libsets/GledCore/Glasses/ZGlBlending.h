// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlBlending_H
#define GledCore_ZGlBlending_H

#include <Glasses/ZRnrModBase.h>
#include <Stones/ZColor.h>

#include <GL/gl.h>

class ZGlBlending : public ZRnrModBase {
  // 7777 RnrCtrl(RnrBits(0,4,0,0))
  MAC_RNR_FRIENDS(ZGlBlending);

private:
  void _init();

protected:
  // Blend mode
  // missing support for glBlendFuncSeparate
  ZRnrModBase::Operation_e
                mBlendOp;     // X{GS}  7 PhonyEnum()
  Int_t		mBSrcFac;     // X{GS}  7 PhonyEnum(-width=>11, -vals=>[GL_ZERO,Zero, GL_ONE,One, GL_DST_COLOR,DstColor, GL_ONE_MINUS_DST_COLOR,OMDstColor, GL_SRC_ALPHA,SrcAlpha, GL_ONE_MINUS_SRC_ALPHA,OMSrcAlpha, GL_DST_ALPHA,DstAplha, GL_ONE_MINUS_DST_ALPHA,OMDstAlpha, GL_SRC_ALPHA_SATURATE,SrcAlphaSaturate, GL_CONSTANT_COLOR,ConstColor, GL_ONE_MINUS_CONSTANT_COLOR,OMConstCol, GL_CONSTANT_ALPHA,ConstAlpha, GL_ONE_MINUS_CONSTANT_ALPHA,OMConstAlpha], -join=>1)
  Int_t		mBDstFac;     // X{GS}  7 PhonyEnum(-width=>10, -vals=>[GL_ZERO,Zero, GL_ONE,One, GL_SRC_COLOR,SrcColor, GL_ONE_MINUS_SRC_COLOR,OMSrcColor, GL_SRC_ALPHA,SrcAlpha, GL_ONE_MINUS_SRC_ALPHA,OMSrcAlpha, GL_DST_ALPHA,DstAplha, GL_ONE_MINUS_DST_ALPHA,OMDstAlpha, GL_CONSTANT_COLOR,ConstColor, GL_ONE_MINUS_CONSTANT_COLOR,OMConstCol, GL_CONSTANT_ALPHA,ConstAlpha, GL_ONE_MINUS_CONSTANT_ALPHA,OMConstAlpha])
  Int_t		mBEquation;   // X{GS}  7 PhonyEnum(-width=>10, -vals=>[GL_FUNC_ADD,Add, GL_FUNC_SUBTRACT,Sub, GL_FUNC_REVERSE_SUBTRACT,ReverseSub, GL_MIN,Min, GL_MAX,Max], -join=>1)
  ZColor	mBConstCol;   // X{PGS} 7 ColorButt()

  // Antialiasing points, lines
  ZRnrModBase::Operation_e
                mAntiAliasOp; // X{GS}  7 PhonyEnum()
  Bool_t	bPointSmooth; // X{GS}  7 Bool(-join=>1)
  Float_t	mPointSize;   // X{GS}  7 Value(-width=>5, -range=>[0.01,64,1,100], -join=>1)
  Int_t		mPointHint;   // X{GS}  7 PhonyEnum(-width=>6, -vals=>[GL_NICEST,Nicest, GL_FASTEST,Fastest, GL_DONT_CARE,DontCare])
  Bool_t	bLineSmooth;  // X{GS}  7 Bool(-join=>1)
  Float_t	mLineWidth;   // X{GS}  7 Value(-width=>4, -range=>[0.01,128,1,100], -join=>1)
  Int_t		mLineHint;    // X{GS}  7 PhonyEnum(-width=>6, -vals=>[GL_NICEST,Nicest, GL_FASTEST,Fastest, GL_DONT_CARE,DontCare])

  // Fog
  ZRnrModBase::Operation_e
                mFogOp;       // X{GS}  7 PhonyEnum()
  Int_t		mFogMode;     // X{GS}  7 PhonyEnum(-width=>6, -vals=>[GL_EXP,Exp, GL_EXP2,Exp2, GL_LINEAR,Linear], -join=>1)
  Int_t		mFogHint;     // X{GS}  7 PhonyEnum(-width=>6, -vals=>[GL_NICEST,Nicest, GL_FASTEST,Fastest, GL_DONT_CARE,DontCare])
  ZColor	mFogColor;    // X{PGS} 7 ColorButt(-join=>1)
  Float_t	mFogDensity;  // X{GS}  7 Value(-range=>[0,1000,1,1000])
  Float_t	mFogBeg;      // X{GS}  7 Value(-range=>[-1000,1000,1,100], -join=>1)
  Float_t	mFogEnd;      // X{GS}  7 Value(-range=>[-1000,1000,1,100])

  // Depth-buffer
  // Missing support for glDepthRange
  ZRnrModBase::Operation_e
                mDepthOp;     // X{GS}  7 PhonyEnum()
  Int_t         mDepthFunc;   // X{GS}  7 PhonyEnum(-vals=>[GL_NEVER,Never, GL_LESS,Less, GL_EQUAL,Equal, GL_LEQUAL,LessEqual, GL_GREATER,Greater, GL_NOTEQUAL,NotEqual, GL_GEQUAL,GreaterEqual, GL_ALWAYS,Always]);
  ZRnrModBase::Operation_e
                mDepthMaskOp; // X{GS}  7 PhonyEnum()
public:
  ZGlBlending(const Text_t* n="ZGlBlending", const Text_t* t=0) : ZRnrModBase(n,t) { _init(); }


#include "ZGlBlending.h7"
  ClassDef(ZGlBlending, 1); // Control of GL blending, anti-aliasing of points & lines and fog
}; // endclass ZGlBlending


#endif
