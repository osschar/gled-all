// $Id: Rnr_GL_SKEL.h 2220 2009-07-09 21:20:00Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_GrowingPlant_GL_RNR_H
#define Tmp1_GrowingPlant_GL_RNR_H

#include "TAttBBox.h"
#include <Glasses/GrowingPlant.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class GrowingPlant_GL_Rnr : public ZNode_GL_Rnr, public TAttBBox
{
protected:
  void _init();
  
  struct Turtle {
    ZTrans mTrans;
  };
  
  GrowingPlant*	    mModel;
  GLUquadricObj*    mQuadric;
  bool              mLighting;
  
  bool              mIAsForward;
  
  virtual void ProcessExpression(RnrDriver* rd);
  virtual void DrawStep(Turtle& turtle, GrowingPlant::Segment& p);
  virtual void DrawSymbol(Turtle& turtle, GrowingPlant::Segment& p) ;
  
  virtual void ComputeBBox() {}  
public:
  GrowingPlant_GL_Rnr(GrowingPlant* idol);
  virtual ~GrowingPlant_GL_Rnr();
  
  virtual void Render(RnrDriver* rd);
  virtual void Triangulate(RnrDriver* rd);
  
  
  virtual void HandlePick(RnrDriver* rd, lNSE_t& ns, lNSE_i nsi);

}; // endclass GrowingPlant_GL_Rnr

#endif
