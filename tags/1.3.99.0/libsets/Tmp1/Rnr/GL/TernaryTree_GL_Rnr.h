// $Id: Rnr_GL_SKEL.h 2220 2009-07-09 21:20:00Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_TernaryTree_GL_RNR_H
#define Tmp1_TernaryTree_GL_RNR_H

#include <Glasses/TernaryTree.h>
#include <Rnr/GL/MonopodialTree_GL_Rnr.h>

class TernaryTree_GL_Rnr : public MonopodialTree_GL_Rnr
{
private:
  void _init();

protected:
  TernaryTree*	mTT;
  virtual void DrawStep(Turtle& turtle, TwoParam& p, bool);
  virtual void ProcessExpression(bool);
  
public:
  TernaryTree_GL_Rnr(TernaryTree* idol);
  virtual ~TernaryTree_GL_Rnr();

}; // endclass TernaryTree_GL_Rnr

#endif
