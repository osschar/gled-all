// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TabletImage_GL_Rnr.h"
#include <GL/gl.h>

/**************************************************************************/

void TabletImage_GL_Rnr::_init()
{
  mTouchStamp = 1;
}

/**************************************************************************/

void TabletImage_GL_Rnr::PreDraw(RnrDriver* rd)
{
  ZImage_GL_Rnr::PreDraw(rd);

  TabletImage& TI = *mTabletImage;
  if(mTouchStamp < TI.mTouchStamp) {
    printf("Applying texture ...\n");

    if(TI.mBrush != 0) {
      ZImage& B = **TI.mBrush;
      TabletImage::TouchInfo& T = TI.mTouch;

      // Enable antialiasing
      // Setup pixel zoom

      /*
      glBlendFunc();
      glBlendEquation();
      glEnable(GL_BLEND);
      */

      glTexSubImage2D(GL_TEXTURE_2D, 0, T.fX, T.fY,
                      B.GetW(), B.GetH(),
                      B.gl_format(), B.gl_type(), B.data());

    }

    mTouchStamp = TI.mTouchStamp;
  }
}
