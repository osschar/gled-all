// $Id: Rnr_GL_SKEL.h 2220 2009-07-09 21:20:00Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_DibotryoidHerb_GL_RNR_H
#define Tmp1_DibotryoidHerb_GL_RNR_H

#include <Glasses/DibotryoidHerb.h>
#include <Rnr/GL/GrowingPlant_GL_Rnr.h>

class DibotryoidHerb_GL_Rnr : public GrowingPlant_GL_Rnr
{
private:
  void _init();

protected:
  DibotryoidHerb*	mDibotryoidHerb;
  
  virtual void DrawSymbol(Turtle& turtle, GrowingPlant::Segment& p);
  virtual void DrawStep(Turtle& turtle, GrowingPlant::Segment& p);
  
public:
  DibotryoidHerb_GL_Rnr(DibotryoidHerb* idol);
  virtual ~DibotryoidHerb_GL_Rnr();
  
    
/*
  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);
 virtual void Render(RnrDriver* rd);
 
 */

}; // endclass DibotryoidHerb_GL_Rnr

#endif
