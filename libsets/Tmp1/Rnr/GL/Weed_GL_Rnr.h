// $Id: Rnr_GL_SKEL.h 2220 2009-07-09 21:20:00Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_Weed_GL_RNR_H
#define Tmp1_Weed_GL_RNR_H

#include <Glasses/Weed.h>
#include <Glasses/ZVector.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

#include <stack>
#include <vector>
#include <map>

class Weed_GL_Rnr : public ZNode_GL_Rnr
{
protected:
  class Turtle {
  public:
    ZTrans   trans;
    float    lineWidth;
    Turtle() : lineWidth(3.f) {}
  };
  
  void _init();  
  
  void ProcessExpression() const;
  
  virtual void SetStepSize();
  virtual void DrawStep(Turtle& t) const;
  virtual void DrawLeaf(Turtle& t) const;
  virtual void DrawFlower(Turtle& t) const;
  virtual void DecreaseWidth(Turtle& t) const;
    
  Weed*	       mWeed;
  float        mStepSize;
  
public:
  Weed_GL_Rnr(Weed* idol);
  virtual ~Weed_GL_Rnr();
  
  virtual void Render(RnrDriver* rd);
  virtual void Triangulate(RnrDriver* rd);
  
  void DumpInfo() const;
  
}; // endclass Weed_GL_Rnr

#endif
