// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_TabletReader_GL_RNR_H
#define Tmp1_TabletReader_GL_RNR_H

#include <Glasses/TabletReader.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class TabletReader_GL_Rnr : public ZNode_GL_Rnr
{
private:
  void _init();

protected:
  TabletReader*	mTabletReader;

public:
  TabletReader_GL_Rnr(TabletReader* idol);
  virtual ~TabletReader_GL_Rnr();

  //virtual void PreDraw(RnrDriver* rd);
  //virtual void Draw(RnrDriver* rd);
  //virtual void PostDraw(RnrDriver* rd);

  virtual void Render(RnrDriver* rd);

}; // endclass TabletReader_GL_Rnr

#endif
