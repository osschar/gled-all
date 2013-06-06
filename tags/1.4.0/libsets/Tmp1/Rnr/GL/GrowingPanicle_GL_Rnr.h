// $Id: Rnr_GL_SKEL.h 2220 2009-07-09 21:20:00Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_GrowingPanicle_GL_RNR_H
#define Tmp1_GrowingPanicle_GL_RNR_H

#include <Glasses/GrowingPanicle.h>
#include <Rnr/GL/GrowingPlant_GL_Rnr.h>

class GrowingPanicle_GL_Rnr : public GrowingPlant_GL_Rnr
{
private:
  void _init();

protected:
  GrowingPanicle*	mGrowingPanicle;
  virtual void DrawSymbol(Turtle& turtle, GrowingPlant::Segment& p);
  virtual void DrawStep(Turtle& turtle, GrowingPlant::Segment& p);
  
  
  virtual void DrawSignal(float);
  
public:
  GrowingPanicle_GL_Rnr(GrowingPanicle* idol);
  virtual ~GrowingPanicle_GL_Rnr();

}; // endclass GrowingPanicle_GL_Rnr

#endif
