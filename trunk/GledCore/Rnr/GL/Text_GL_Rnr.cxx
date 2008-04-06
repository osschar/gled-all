// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Text_GL_Rnr.h"
#include <Rnr/GL/ZRlFont_GL_Rnr.h>
#include <Rnr/GL/GLRnrDriver.h>
#include <GL/gl.h>

#define PARENT ZNode_GL_Rnr

/**************************************************************************/

void Text_GL_Rnr::_init()
{}

/**************************************************************************/

void Text_GL_Rnr::Draw(RnrDriver* rd)
{
  obtain_rnrmod(rd, mFontRMS);
  PARENT::Draw(rd);
}

void Text_GL_Rnr::Render(RnrDriver* rd)
{
  GLTextNS::TexFont *txf = ((ZRlFont_GL_Rnr*)mFontRMS.rnr())->GetFont();

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
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  } else {
    glDisable(GL_BLEND);
  }

  int width, ascent, descent;
  GLTextNS::txfGetStringMetrics(txf, mText->mText.Data(), mText->mText.Length(),
				width, ascent, descent);
  if(mText->bAbsSize) {
    ascent  = txf->max_ascent;
    descent = txf->max_descent;
  }
  int   h_box = ascent + descent;
  float scale = 1.0/h_box;

  glPushMatrix();
  if(mText->bCenter) {
    glTranslatef(-width/2.0*scale, 0, 0);
  }

  Float_t poly_offa = 0;

  if(mText->bBackPoly || mText->bFramePoly) {

    float x0 = -mText->mXBorder;
    float x1 = (float)width/h_box + mText->mXBorder;
    float y0 = -mText->mYBorder   - float(descent)/(h_box);
    float y1 =  mText->mYBorder   + float(ascent)/(h_box);

    if(mText->bFramePoly) {
      rd->GL()->Color(mText->mFrameCol);
      Float_t w = mText->mFrameW;
      glBegin(GL_QUADS);
      glVertex2f(x0-w, y0-w);  glVertex2f(x1+w, y0-w);
      glVertex2f(x1+w, y1+w);  glVertex2f(x0-w, y1+w);
      glEnd();
      poly_offa -= 1;
    }

    if(poly_offa != 0) {
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(poly_offa, poly_offa);
    }

    if(mText->bBackPoly) {
      rd->GL()->Color(mText->mBGCol);
      glBegin(GL_QUADS);
      glVertex2f(x0, y0);  glVertex2f(x1, y0);
      glVertex2f(x1, y1);  glVertex2f(x0, y1);
      glEnd();
      poly_offa -= 1;
    }
  }

  if(poly_offa != 0) {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(poly_offa, poly_offa);
  }

  rd->GL()->Color(mText->mFGCol);
  glScalef(scale, scale, 1);
  glEnable(GL_TEXTURE_2D);
  GLTextNS::txfBindFontTexture(txf);
  GLTextNS::txfRenderString(txf, mText->mText.Data(), mText->mText.Length());

  glPopMatrix();

  glPopAttrib();
}
