// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Text_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <FL/gl.h>

/**************************************************************************/

void Text_GL_Rnr::_init()
{}

/**************************************************************************/

void Text_GL_Rnr::Draw(RnrDriver* rd)
{
  if(rd->fTexFont) { // TexFont test
    ZGlass_GL_Rnr::Draw(rd);
  }
}

void Text_GL_Rnr::Render(RnrDriver* rd)
{
  GLTextNS::TexFont *txf = rd->fTexFont;

  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT);

  glNormal3f(0, 0, 1);

  glPolygonMode(GL_FRONT, GL_FILL);
  if(mText->bFillBack) {
    glPolygonMode(GL_BACK, GL_FILL);
  }

  if(mText->bAlpha) {
    glAlphaFunc(GL_GEQUAL, 0.0625);
    glEnable(GL_ALPHA_TEST);
  } else {
    glDisable(GL_ALPHA_TEST);
  }
  if(mText->bBlend) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else {
    glDisable(GL_BLEND);
  }

  int width, ascent, descent;
  GLTextNS::txfGetStringMetrics(txf, mText->mText.Data(), mText->mText.Length(),
				&width, &ascent, &descent);
  if(mText->bAbsSize) {
    ascent  = txf->max_ascent;
    descent = txf->max_descent;
  }
  int   h_box = ascent + descent;
  float scale = 1.0/h_box;

  if(mText->bBorder) {

    if(mText->bPolyOffBG) {
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(mText->mPOFac/2, mText->mPOUni/2);
    } else {
      glDisable(GL_POLYGON_OFFSET_FILL);
    }
      
    float x0 = -mText->mXBorder;
    float x1 = (float)width/h_box + mText->mXBorder;
    float y0 = -mText->mYBorder   - float(descent)/(h_box);
    float y1 =  mText->mYBorder   + float(ascent)/(h_box);
    glColor4fv(mText->mBGCol());
    glBegin(GL_QUADS);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();

  }

  if(mText->bPolyOffFG) {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(mText->mPOFac, mText->mPOUni);
  } else {
    glDisable(GL_POLYGON_OFFSET_FILL);
  }

  glColor4fv(mText->mFGCol());
  glPushMatrix();
  glScalef(scale, scale, 1);
  glEnable(GL_TEXTURE_2D);
  GLTextNS::txfBindFontTexture(txf);
  GLTextNS::txfRenderString(txf, mText->mText.Data(), mText->mText.Length());
  glPopMatrix();

  glPopAttrib();
}
