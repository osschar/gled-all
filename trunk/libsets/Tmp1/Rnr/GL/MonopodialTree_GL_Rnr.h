// $Id: Rnr_GL_SKEL.h 2220 2009-07-09 21:20:00Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_MonopodialTree_GL_RNR_H
#define Tmp1_MonopodialTree_GL_RNR_H

#include <Glasses/MonopodialTree.h>
#include <Rnr/GL/ParametricSystem_GL_Rnr.h>

class MonopodialTree_GL_Rnr : public ParametricSystem_GL_Rnr
{
private:
  void _init();

protected:
  MonopodialTree*	mMonopodialTree;

public:
  MonopodialTree_GL_Rnr(MonopodialTree* idol);
  virtual ~MonopodialTree_GL_Rnr();
/*
  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

  virtual void Render(RnrDriver* rd);
  
 */
  
  virtual void Triangulate(RnrDriver* rd);
}; // endclass MonopodialTree_GL_Rnr

#endif
