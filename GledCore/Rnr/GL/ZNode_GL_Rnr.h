// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZNode_GL_Rnr
#define Gled_ZNode_GL_Rnr

#include <Glasses/ZNode.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>
class RnrDriver;

class ZNode_GL_Rnr : public ZGlass_GL_Rnr {

protected:
  ZNode*	mNode;
  TimeStamp_t	mStampTrans;
  float		mGL_Mat[16];

  void build_GL_mat();

public:
  ZNode_GL_Rnr(ZNode* n) : ZGlass_GL_Rnr(n), mNode(n), mStampTrans(0) {}

  virtual void PreDraw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass ZNode_GL_Rnr

#endif
