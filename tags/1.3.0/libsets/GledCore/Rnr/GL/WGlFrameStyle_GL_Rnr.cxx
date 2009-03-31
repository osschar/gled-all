// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "WGlFrameStyle_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>
#include <Rnr/GL/GLTextNS.h>

#include <GL/glew.h>

#define PARENT ZRnrModBase_GL_Rnr

/**************************************************************************/

void WGlFrameStyle_GL_Rnr::_init()
{}

/**************************************************************************/

void WGlFrameStyle_GL_Rnr::PreDraw(RnrDriver* rd)
{
  PARENT::PreDraw(rd);
  update_tring_stamp(rd);
  rd->PushRnrMod(WGlFrameStyle::FID(), mRnrMod);
}

void WGlFrameStyle_GL_Rnr::Draw(RnrDriver* rd)
{
  update_tring_stamp(rd);
  rd->SetDefRnrMod(WGlFrameStyle::FID(), mRnrMod);
}

void WGlFrameStyle_GL_Rnr::PostDraw(RnrDriver* rd)
{
  rd->PopRnrMod(WGlFrameStyle::FID());
  PARENT::PostDraw(rd);
}

/**************************************************************************/

void WGlFrameStyle_GL_Rnr::BoxLimits(float& dx, float& dy,
				     float&  x, float&  y, float& w, float& h)
{
  // Sets-up full-frame extent (dx, dy) and inner region limits (x,y,w,h).

  WGlFrameStyle& FS = *mWGlFrameStyle;
  if(dx == 0) dx = FS.mDefDx;
  if(dy == 0) dy = FS.mTextYSize + 2*FS.mYBorder;
  x = FS.mXBorder;
  y = FS.mYBorder;
  w = dx - 2*FS.mXBorder;
  h = dy - 2*FS.mYBorder;
}

void WGlFrameStyle_GL_Rnr::StudyText(GLTextNS::TexFont *txf, TString& label,
				     float& scale,
				     float& x, float& y, float& w, float& h)
{
  // Places text in the given box. Returns text-box and sets scale to
  // be used for text drawing.

  WGlFrameStyle& FS = *mWGlFrameStyle;

  int width, ascent, descent;
  GLTextNS::txfGetStringMetrics(txf, label.Data(), label.Length(),
				width, ascent, descent);
  ascent  = txf->max_ascent;
  descent = txf->max_descent;

  float text_h  = FS.mTextYSize;
  bool  fullh_p = false;
  if(h < text_h) {
    text_h  = h;
    fullh_p = true;
  }
  int   height   = ascent + descent;
        scale    = text_h / height;
  float scaled_w = width  * scale;
  bool  fullw_p  = false;
  if(scaled_w > w) {
    scaled_w = w;
    fullw_p  = true;
  }

  if(!fullw_p) {
    switch(FS.mHAlign) {
    case WGlFrameStyle::HA_Center: x += (w-scaled_w)/2; break;
    case WGlFrameStyle::HA_Right:  x += (w-scaled_w);   break;
    default: break;
    }
  }
  if(!fullh_p) {
    switch(FS.mVAlign) {
    case WGlFrameStyle::VA_Center: y += (h - text_h)/2; break;
    case WGlFrameStyle::VA_Top:    y += (h - text_h);   break;
    default: break;
    }
  }
  y += descent*scale;
  w = scaled_w;
  h = text_h;
}

/**************************************************************************/

void WGlFrameStyle_GL_Rnr::RenderTile(float dx, float dy, bool belowmouse, const ZColor* col)
{
   WGlFrameStyle& FS = *mWGlFrameStyle;
   if(FS.bDrawTile) {
     if(belowmouse) {
       glColor4fv(FS.mBelowMColor());
     } else {
       if(col) glColor4fv((*col)());
       else         glColor4fv(FS.mTileColor());
     }
     glBegin(GL_QUADS);
     glVertex2f(0, 0);   glVertex2f(dx, 0);
     glVertex2f(dx, dy); glVertex2f(0, dy);
     glEnd();
   }
}

void WGlFrameStyle_GL_Rnr::RenderFrame(float dx, float dy)
{
  WGlFrameStyle& FS = *mWGlFrameStyle;
   if(FS.bDrawFrame) {
     glColor4fv(FS.mFrameColor());
     if(FS.mFrameW) glLineWidth(FS.mFrameW);
     glBegin(GL_LINE_LOOP);
     glVertex2f(0, 0);   glVertex2f(dx, 0);
     glVertex2f(dx, dy); glVertex2f(0, dy);
     glEnd();
   }
}

void WGlFrameStyle_GL_Rnr::RenderText(GLTextNS::TexFont *txf, TString& label,
				      float scale,
				      float x, float y, float max_w)
{
  WGlFrameStyle& FS = *mWGlFrameStyle;

  glPushMatrix();
  glTranslatef(x, y, 0);

  glScalef(scale, scale, 1);
  glColor4fv(FS.mTextColor());
  GL_Capability_Switch texure_on(GL_TEXTURE_2D, true);
  GLTextNS::txfBindFontTexture(txf);
  txfRenderString(txf, label.Data(), label.Length(),
		  max_w/scale, FS.mTextFadeW);

  glPopMatrix();
}

/**************************************************************************/

void WGlFrameStyle_GL_Rnr::FullRender(GLTextNS::TexFont *txf, TString& label,
				      float dx, float dy, bool belowmouse)
{
  WGlFrameStyle& FS = *mWGlFrameStyle;

  glPushAttrib(GL_COLOR_BUFFER_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  float x, y, w, h, scale, xt, yt, wt, ht;
  BoxLimits(dx, dy, x, y, w, h);
  xt = x; yt = y; wt = w; ht = h;

  StudyText(txf, label, scale, xt, yt, wt, ht);
  if(FS.bMinTile && FS.mHAlign == WGlFrameStyle::HA_Left)
    dx -= (x + w) - (xt + wt);

  glNormal3f(0, 0, 1);
  glEnable(GL_POLYGON_OFFSET_FILL);

  glPolygonOffset(1, 1);
  RenderTile(dx, dy, belowmouse);

  RenderFrame(dx, dy);

  glPolygonOffset(-1, -1);
  RenderText(txf, label, scale, xt, yt, w);

  glDisable(GL_POLYGON_OFFSET_FILL);

  glPopAttrib();
}

/**************************************************************************/

void WGlFrameStyle_GL_Rnr::FullSymbolRender(GLTextNS::TexFont *txf, TString& label,
					    float dx, float dy, bool belowmouse, const ZColor* back_col, const ZColor* sym_color)
{
  WGlFrameStyle& FS = *mWGlFrameStyle;

  glPushAttrib(GL_COLOR_BUFFER_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  float x, y, w, h, scale, xt, yt, wt, ht;
  BoxLimits(dx, dy, x, y, w, h);
  xt = x; yt = y; wt = w; ht = h;

  StudyText(txf, label, scale, xt, yt, wt, ht);
  if(FS.bMinTile && FS.mHAlign == WGlFrameStyle::HA_Left)
    dx -= (x + w) - (xt + wt);

  glNormal3f(0, 0, 1);
  glEnable(GL_POLYGON_OFFSET_FILL);

  glPolygonOffset(1, 1);
  RenderTile(dx, dy, belowmouse, back_col);

  RenderFrame(dx, dy);

  glPolygonOffset(-1, -1);
  if (label == "<" ) {
    render_triangle(1, dx, dy, sym_color);
  } else if (label == "<<" ) {
    render_triangle(2,dx, dy, sym_color);
  } else if (label == ">" ) {
    render_triangle(3,dx, dy, sym_color);
  } else if (label == ">>") {
    render_triangle(4,dx, dy, sym_color);
  }
  else {
    RenderText(txf, label, scale, xt, yt, w);
  }
  glDisable(GL_POLYGON_OFFSET_FILL);
  glPopAttrib();
}

/**************************************************************************/
void WGlFrameStyle_GL_Rnr::render_triangle(int id, float dx, float dy, const ZColor* col){
  glColor4fv((*col)());
  float xs = dx/4, ys = dy/4;

  glBegin(GL_TRIANGLES);
  switch (id) {
  case 1:
    {
      glVertex2f(xs*2.5, ys*3); glVertex2f(xs*1.5, ys*2); glVertex2f(xs*2.5, ys);
      break;
    }
  case 2:
    {
      glVertex2f(xs*2, ys*3); glVertex2f(xs, ys*2); glVertex2f(xs*2, ys);
      glVertex2f(xs*3, ys*3); glVertex2f(xs*2, ys*2); glVertex2f(xs*3, ys);
      break;
    }
  case 3:
    {
      glVertex2f(xs*1.5, ys);  glVertex2f(xs*2.5, ys*2); glVertex2f(xs*1.5, ys*3);
      break;
    }
  case 4:
    {
      glVertex2f(xs, ys);  glVertex2f(xs*2, ys*2); glVertex2f(xs, ys*3);
      glVertex2f(xs*2, ys); glVertex2f(xs*3, ys*2); glVertex2f(xs*2, ys*3);
      break;
    }
  default:
    break;
  }
  glEnd();
}
