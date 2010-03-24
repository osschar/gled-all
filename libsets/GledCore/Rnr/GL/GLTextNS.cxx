// $Id$

// The following implementation is based on TexFont API,
// implementation and accompanying programs by Mark J. Kilgard.
// Original license:

/* Copyright (c) Mark J. Kilgard, 1997. */
/* This program is freely distributable without licensing fees  and is
   provided without guarantee or warrantee expressed or implied. This
   program is -not- in the public domain. */

#include "GLTextNS.h"

#include <Rnr/GL/GLRnrDriver.h>
#include <Rnr/GL/ZRlFont_GL_Rnr.h>
#include <Rnr/GL/ZRlNodeMarkup_GL_Rnr.h>

#include <TMath.h>

// Use ftgl-2.1.2 shipped with root. In 2.1.3 one should include "FTGL/ftgl.h"
#include "FTFont.h"
//#include "FTGLExtrdFont.h"
//#include "FTGLOutlineFont.h"
//#include "FTGLPolygonFont.h"
//#include "FTGLTextureFont.h"
//#include "FTGLPixmapFont.h"
//#include "FTGLBitmapFont.h"

//==============================================================================

GLTextNS::TextLineData::TextLineData(FTFont *ftf, const TString& line) :
  text(line)
{
  width = MeasureWidth(ftf, line, ascent, descent);
  hfull = ftf->LineHeight();
}

//==============================================================================

Float_t GLTextNS::MeasureWidth(FTFont *ftf, const TString& txt)
{
  return ftf->Advance(txt);
}

Float_t GLTextNS::MeasureWidth(FTFont *ftf, const TString& txt,
			       Float_t& ascent, Float_t& descent)
{
  ascent  =   ftf->Ascender();
  descent = - ftf->Descender();
  return ftf->Advance(txt);
}

//==============================================================================

void GLTextNS::RnrTextBar(RnrDriver* rd, const TString& text)
{
  RNRDRIVER_GET_RNRMOD_LENS(nrc, rd, ZRlNodeMarkup);
  BoxSpecs boxs;
  boxs.pos = nrc_lens->RefTilePos();
  RnrTextBar(rd, text, boxs, nrc_lens->GetNameOffset());
}

void GLTextNS::RnrTextBar(RnrDriver* rd, const TString& text,
			  BoxSpecs& bs, float zoffset)
{
  // RasterPos screws-up picking ... but so does gluProject.
  /*
    GLdouble pm[16], mm[16], wx, wy, wz;
    GLint    vp[4];
    glGetDoublev(GL_PROJECTION_MATRIX, pm);
    glGetDoublev(GL_MODELVIEW_MATRIX,  mm);
    glGetIntegerv(GL_VIEWPORT, vp);

    glColor3f(0,1,0); glBegin(GL_POINTS); glVertex3f(-.3, 0, 0); glEnd();

    int ret = gluProject(0, 0, 0, mm, pm, vp, &wx, &wy, &wz);
    if(wx < vp[0] || wx > vp[0]+vp[2]) return;
    if(wy < vp[1] || wy > vp[1]+vp[3]) return;
    if(wz < 0     || wz > 1)           return;
  */

  GLboolean rv;
  glRasterPos4f(0, 0, 0, 1);
  glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &rv);
  if(rv == false) return;

  typedef list<TextLineData>           lTLD_t;
  typedef list<TextLineData>::iterator lTLD_i;

  RNRDRIVER_GET_RNRMOD_LENS(nrc, rd, ZRlNodeMarkup);
  RNRDRIVER_GET_RNRMOD_BOTH(font, rd, ZRlFont);
  FTFont *txf = font_rnr->GetFont();

  lStr_t lines;
  lTLD_t tlds;
  float
    max_width = 0,
    width     = 0,
    ascent    =   txf->Ascender(),
    descent   = - txf->Descender(),
    height    = 0,
    interline = ascent + descent + bs.lineskip;

  GledNS::split_string(text, lines, '\n');
  for(lStr_i l=lines.begin(); l!=lines.end(); ++l) {
    tlds.push_back( TextLineData(txf, *l) );
    TextLineData& tld = tlds.back();
    max_width = TMath::Max(max_width, tld.width);
    height += interline;
  }
  height += bs.tm + descent + bs.bm;
  width   = max_width + bs.lm + bs.rm;
  float halfw = float(width)/2, halfh = float(height)/2;

  float tsize = float(font_lens->GetSize());
  float scale = tsize / ascent;

  // printf("%d = %d + %d; %f %f\n", descent+ascent, ascent, descent, tsize, scale);

  glPushAttrib(GL_TEXTURE_BIT   |
	       GL_LINE_BIT      | GL_COLOR_BUFFER_BIT |
	       GL_POLYGON_BIT);

  GLfloat rp[4];
  glGetFloatv(GL_CURRENT_RASTER_POSITION, rp);
  // printf("RasterPos[%s]: %f %f %f %f\n", text.Data(), rp[0], rp[1], rp[2], rp[3]);

  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadMatrixd(rd->GetProjBase()->Array());
  glOrtho(0, rd->GetWidth(), 0, rd->GetHeight(), 0, -1);
  glMatrixMode(GL_MODELVIEW);

  // Translate to required position.
  float
    xo = rp[0] - halfw,
    yo = rp[1] + halfh,
    zo = rp[2] * zoffset;
  for(Ssiz_t i=0; i<bs.pos.Length(); ++i) {
    switch(bs.pos(i)) {
    case 'l': case 'L': xo += halfw; break;
    case 'r': case 'R': xo -= halfw; break;
    case 't': case 'T': yo -= halfh; break;
    case 'b': case 'B': yo += halfh; break;
    }
  }
  glTranslatef(TMath::Nint(xo), TMath::Nint(yo), zo);

  glScalef(scale, scale, 1);

  if(nrc_lens->GetRnrTiles()) {
    glColor4fv(nrc_lens->RefTileCol()());
    glBegin(GL_QUADS);
    glVertex2i(0, -height); glVertex2i(width, -height);
    glVertex2i(width, 0);   glVertex2i(0, 0);
    glEnd();
  }

  glColor4fv(nrc_lens->RefTextCol()());
  glTranslatef(0, 0, -1e-6);

  if(nrc_lens->GetRnrFrames()) {
    glLineWidth(1);
    glBegin(GL_LINE_LOOP);
    glVertex2i(0, -height); glVertex2i(width, -height);
    glVertex2i(width, 0);   glVertex2i(0, 0);
    glEnd();
  }

  glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_TEXTURE_2D);

  glTranslatef(bs.lm, -(bs.tm + descent + ascent), 0);
  for(lTLD_i l=tlds.begin(); l!=tlds.end(); ++l) {
    glPushMatrix();
    if(l->width != max_width) {
      switch(bs.align) {
      case 'c': case 'C': glTranslatef((max_width - l->width)/2, 0, 0); break;
      case 'r': case 'R': glTranslatef((max_width - l->width),   0, 0); break;
      }
    }
    txf->Render(l->text);
    glPopMatrix();
    glTranslatef(0, -interline, 0);
  }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glPopAttrib();
}

/**************************************************************************/

void GLTextNS::RnrTextPoly(RnrDriver* rd, const TString& text)
{
  RNRDRIVER_GET_RNRMOD_RNR(font, rd, ZRlFont);
  FTFont *txf = font_rnr->GetFont();
  RNRDRIVER_GET_RNRMOD_LENS(nrc, rd, ZRlNodeMarkup);

  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT);

  glNormal3f(0, 0, 1);

  glPolygonMode(GL_FRONT, GL_FILL);
  glDisable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  float width, ascent, descent;
  width = MeasureWidth(txf, text, ascent, descent);
  float   h_box = ascent + descent;
  float scale = 1.0/h_box;


  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(-1, -1);

  float x0 = -0.1;
  float x1 = (float)width/h_box + 0.1;
  float y0 = -0.1 - float(descent)/(h_box);
  float y1 =  0.1 + float(ascent)/(h_box);
  glColor4fv(nrc_lens->RefTileCol()());

  if(nrc_lens->GetRnrTiles()) {
    glBegin(GL_QUADS);
    glVertex2f(x0, y0); glVertex2f(x1, y0);
    glVertex2f(x1, y1); glVertex2f(x0, y1);
    glEnd();
  }

  glPolygonOffset(-2, -2);

  if(nrc_lens->GetRnrFrames()) {
    glBegin(GL_LINE_LOOP);
    glVertex2f(x0, y0); glVertex2f(x1, y0);
    glVertex2f(x1, y1); glVertex2f(x0, y1);
    glEnd();
  }

  glColor4fv(nrc_lens->RefTextCol()());
  glPushMatrix();
  glScalef(scale, scale, 1);
  glEnable(GL_TEXTURE_2D);
  txf->Render(text);
  glPopMatrix();

  glPopAttrib();
}

/**************************************************************************/

void GLTextNS::RnrText(RnrDriver* rd, const TString& text,
		       int x, int y, float z,
		       const ZColor* front_col, const ZColor* back_col)
{
  // Renders string text on position specified by:
  // x - num pixels from the left border (right if x < 0)
  // y - pixels from bottom border
  // z - z coordinate in depth-buffer coordinates (0 -> 1)
  // If front_col == 0 renders uses white pen.
  // If back_col  != 0 renders a square of that color behind the text.

  RNRDRIVER_GET_RNRMOD_BOTH(font, rd, ZRlFont);
  FTFont *txf = font_rnr->GetFont();

  glPushAttrib(GL_TEXTURE_BIT      |
	       GL_LIGHTING_BIT     |
	       GL_COLOR_BUFFER_BIT |
	       GL_POLYGON_BIT);


  glPolygonMode(GL_FRONT, GL_FILL);
  glDisable(GL_LIGHTING);
  glDisable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  float width, ascent, descent;
  width = MeasureWidth(txf, text, ascent, descent);
  // int h_box = ascent + descent;
  float scale = float(font_lens->GetSize()) / ascent;

  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadMatrixd(rd->GetProjBase()->Array());
  glOrtho(0, rd->GetWidth(), 0, rd->GetHeight(), 0, -1);

  if(x < 0)
    x = rd->GetWidth() - TMath::Nint(width*scale) + x;
  glTranslatef(x, (y - descent)*scale, z);
  glScalef(scale, scale, 1);

  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(-1, -1);

  if(back_col != 0) {
    float x0 = -1;
    float x1 = width + 1;
    float y0 = -descent;
    float y1 = ascent;
    glColor4fv((*back_col)());
    glBegin(GL_QUADS);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();
  }

  glPolygonOffset(-2, -2);
  if(front_col == 0) glColor3f(1, 1, 1); else glColor4fv((*front_col)());
  glEnable(GL_TEXTURE_2D);

  txf->Render(text);

  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glPopAttrib();
}

/**************************************************************************/

void GLTextNS::RnrTextAt(RnrDriver* rd, const TString& text,
			 int x, int yrow, float z,
                         const ZColor* front_col, const ZColor* back_col)
{
  // Renders TString text on position specified by:
  // x    - num pixels from the left border (right if x < 0)
  // yrow - line number in up-down order; 0 is the first/top line
  //        and from bottom-up for yrow < 0.
  // z    - z coordinate in depth-buffer coordinates (0 -> 1)
  // If front_col == 0 text is drawn in white.
  // If back_col  != 0 renders a rectangle of that color behind the text.

  RNRDRIVER_GET_RNRMOD_BOTH(font, rd, ZRlFont);
  FTFont *txf = font_rnr->GetFont();

  glPushAttrib(GL_TEXTURE_BIT      |
	       GL_LIGHTING_BIT     |
	       GL_COLOR_BUFFER_BIT |
	       GL_POLYGON_BIT);

  glPolygonMode(GL_FRONT, GL_FILL);
  glDisable(GL_LIGHTING);
  glDisable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  float width, ascent, descent;
  width = MeasureWidth(txf, text, ascent, descent);
  float h_box = ascent + descent;
  float scale = float(font_lens->GetSize()) / ascent;

  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadMatrixd(rd->GetProjBase()->Array());
  glOrtho(0, rd->GetWidth(), 0, rd->GetHeight(), 0, -1);
  glMatrixMode(GL_MODELVIEW);

  int y;
  if(x < 0)
    x = rd->GetWidth() - TMath::Nint(width*scale) + x;
  if (yrow < 0)
    y = TMath::Nint((-(1 + yrow)*h_box + descent) * scale);
  else
    y = TMath::Nint(rd->GetHeight() - (yrow*h_box + ascent) * scale);
  glTranslatef(x, y, z);
  glScalef(scale, scale, 1);

  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(-1, -1);

  if(back_col != 0) {
    float x0 = -1;
    float x1 = width + 1;
    float y0 = -descent;
    float y1 = ascent;
    glColor4fv((*back_col)());
    glBegin(GL_QUADS);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();
  }

  glPolygonOffset(-2, -2);
  if(front_col == 0) glColor3f(1, 1, 1); else glColor4fv((*front_col)());
  glEnable(GL_TEXTURE_2D);

  txf->Render(text);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glPopAttrib();
}
