// $Id: Rnr_GL_SKEL.h 2220 2009-07-09 21:20:00Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_Plant_GL_RNR_H
#define Tmp1_Plant_GL_RNR_H

#include <Glasses/Plant.h>
#include <Rnr/GL/Weed_GL_Rnr.h>

class Plant_GL_Rnr : public Weed_GL_Rnr
{
private:
  void _init();
  Plant*	mPlant;
  GLUquadricObj* mQuadric; 
  
protected:  
  virtual void DrawStep(Turtle& t) const;
  virtual void DrawLeaf(Turtle& t) const;
  virtual void DrawFlower(Turtle& t) const;
  virtual void DecreaseWidth(Turtle& t) const;

  virtual void SetStepSize();
   
public:
  Plant_GL_Rnr(Plant* idol);
  virtual ~Plant_GL_Rnr();
  
  virtual void Render(RnrDriver* rd);
}; // endclass Plant_GL_Rnr

#endif
