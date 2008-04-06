// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TabletImage_GL_RNR_H
#define Var1_TabletImage_GL_RNR_H

#include <Glasses/TabletImage.h>
#include <Rnr/GL/ZImage_GL_Rnr.h>

class TabletImage_GL_Rnr : public ZImage_GL_Rnr {
private:
  void _init();

protected:
  TabletImage*	mTabletImage;
  TimeStamp_t   mTouchStamp;

public:
  TabletImage_GL_Rnr(TabletImage* idol) :
    ZImage_GL_Rnr(idol), mTabletImage(idol)
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);

}; // endclass TabletImage_GL_Rnr

#endif
