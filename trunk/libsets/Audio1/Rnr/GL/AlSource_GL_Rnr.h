// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Audio1_AlSource_GL_RNR_H
#define Audio1_AlSource_GL_RNR_H

#include <Glasses/AlSource.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class AlSource_GL_Rnr : public ZNode_GL_Rnr
{
private:
  void _init();

protected:
  AlSource*	mAlSource;

public:
  AlSource_GL_Rnr(AlSource* idol) :
    ZNode_GL_Rnr(idol), mAlSource(idol)
  { _init(); }

  //virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  //virtual void PostDraw(RnrDriver* rd);

}; // endclass AlSource_GL_Rnr

#endif
