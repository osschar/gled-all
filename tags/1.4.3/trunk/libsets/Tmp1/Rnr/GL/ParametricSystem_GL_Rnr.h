// $Id: Rnr_GL_SKEL.h 2220 2009-07-09 21:20:00Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_ParametricSystem_GL_RNR_H
#define Tmp1_ParametricSystem_GL_RNR_H

#include "TAttBBox.h"
#include <Glasses/ParametricSystem.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class ZTrans;
class TRandom;

class ParametricSystem_GL_Rnr : public ZNode_GL_Rnr,
                                public TAttBBox
{  
private:
  void _init();
  
protected:
  
  struct Turtle {
    ZTrans mTrans;
    float  mWidth;
    float  mLength;
    bool   mChangedWidth;
  };
  
  ParametricSystem*	mPS;
  GLUquadricObj*    mQuadric;
  float             mScale;
  TRandom*          mRandom;
  UInt_t            mSeed;
   
  virtual void ProcessExpression(bool);
  virtual void DrawStep(Turtle& turtle, TwoParam& p, bool);
    
  
  virtual void ComputeBBox() {}
  float GetMaxExtend();

public:
  ParametricSystem_GL_Rnr(ParametricSystem* idol);
  virtual ~ParametricSystem_GL_Rnr();
  
  virtual void Render(RnrDriver* rd);
  virtual void Triangulate(RnrDriver* rd);
  
}; // endclass ParametricSystem_GL_Rnr

#endif
