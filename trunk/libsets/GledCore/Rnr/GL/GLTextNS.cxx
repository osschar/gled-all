// $Header$

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

#include <GL/glu.h>

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

namespace GLTextNS {

#if 0
  /* Uncomment to debug various scenarios. */
#undef GL_VERSION_1_1
#undef GL_EXT_texture_object
#undef GL_EXT_texture
#endif

  int useLuminanceAlpha = 1;

  /* byte swap a 32-bit value */
#define SWAPL(x, n) {				\
    n = ((char *) (x))[0];			\
    ((char *) (x))[0] = ((char *) (x))[3];	\
    ((char *) (x))[3] = n;			\
    n = ((char *) (x))[1];			\
    ((char *) (x))[1] = ((char *) (x))[2];	\
    ((char *) (x))[2] = n; }

  /* byte swap a short */
#define SWAPS(x, n) {				\
    n = ((char *) (x))[0];			\
    ((char *) (x))[0] = ((char *) (x))[1];	\
    ((char *) (x))[1] = n; }

  /**************************************************************************/

  static TexGlyphVertexInfo* getTCVI(TexFont * txf, int c)
  {
    TexGlyphVertexInfo *tgvi;

    /* Automatically substitute uppercase letters with lowercase if not
       uppercase available (and vice versa). */
    if ((c >= txf->min_glyph) && (c < txf->min_glyph + txf->range)) {
      tgvi = txf->lut[c - txf->min_glyph];
      if (tgvi) {
	return tgvi;
      }
      if (islower(c)) {
	c = toupper(c);
	if ((c >= txf->min_glyph) && (c < txf->min_glyph + txf->range)) {
	  return txf->lut[c - txf->min_glyph];
	}
      }
      if (isupper(c)) {
	c = tolower(c);
	if ((c >= txf->min_glyph) && (c < txf->min_glyph + txf->range)) {
	  return txf->lut[c - txf->min_glyph];
	}
      }
    }

    //fprintf(stderr, "texfont: tried to access unavailable font character \"%c\" (%d)\n",
    //    isprint(c) ? c : ' ', c);

    tgvi = txf->lut[' ' - txf->min_glyph];
    if (tgvi) return tgvi;
    tgvi = txf->lut['_' - txf->min_glyph];
    if (tgvi) return tgvi;

    return 0;
  }

  /**************************************************************************/

  static const char *lastError;

  const char* txfErrorString(void)
  {
    return lastError;
  }

  /**************************************************************************/

  TexFont* txfLoadFont(const char *filename)
  {
    TexFont *txf;
    FILE *file;
    GLfloat w, h, xstep, ystep;
    char fileid[4], tmp;
    unsigned char *texbitmap;
    int min_glyph, max_glyph;
    int endianness, swap, format, stride, width, height;
    int i, j, got;

    txf = NULL;
    file = fopen(filename, "rb");
    if (file == NULL) {
      lastError = "file open failed.";
      goto error;
    }
    txf = (TexFont *) malloc(sizeof(TexFont));
    if (txf == NULL) {
      lastError = "out of memory.";
      goto error;
    }
    /* For easy cleanup in error case. */
    txf->texobj = 0; // MT add
    txf->tgi = NULL;
    txf->tgvi = NULL;
    txf->lut = NULL;
    txf->teximage = NULL;

    got = fread(fileid, 1, 4, file);
    if (got != 4 || strncmp(fileid, "\377txf", 4)) {
      lastError = "not a texture font file.";
      goto error;
    }
    assert(sizeof(int) == 4);  /* Ensure external file format size. */
    got = fread(&endianness, sizeof(int), 1, file);
    if (got == 1 && endianness == 0x12345678) {
      swap = 0;
    } else if (got == 1 && endianness == 0x78563412) {
      swap = 1;
    } else {
      lastError = "not a texture font file.";
      goto error;
    }
#define EXPECT(n) if (got != n) { lastError = "premature end of file."; goto error; }
    got = fread(&format, sizeof(int), 1, file);
    EXPECT(1);
    got = fread(&txf->tex_width, sizeof(int), 1, file);
    EXPECT(1);
    got = fread(&txf->tex_height, sizeof(int), 1, file);
    EXPECT(1);
    got = fread(&txf->max_ascent, sizeof(int), 1, file);
    EXPECT(1);
    got = fread(&txf->max_descent, sizeof(int), 1, file);
    EXPECT(1);
    got = fread(&txf->num_glyphs, sizeof(int), 1, file);
    EXPECT(1);

    if (swap) {
      SWAPL(&format, tmp);
      SWAPL(&txf->tex_width, tmp);
      SWAPL(&txf->tex_height, tmp);
      SWAPL(&txf->max_ascent, tmp);
      SWAPL(&txf->max_descent, tmp);
      SWAPL(&txf->num_glyphs, tmp);
    }
    txf->tgi = (TexGlyphInfo *) malloc(txf->num_glyphs * sizeof(TexGlyphInfo));
    if (txf->tgi == NULL) {
      lastError = "out of memory.";
      goto error;
    }
    assert(sizeof(TexGlyphInfo) == 12);  /* Ensure external file format size. */
    got = fread(txf->tgi, sizeof(TexGlyphInfo), txf->num_glyphs, file);
    EXPECT(txf->num_glyphs);

    if (swap) {
      for (i = 0; i < txf->num_glyphs; i++) {
	SWAPS(&txf->tgi[i].c, tmp);
	SWAPS(&txf->tgi[i].x, tmp);
	SWAPS(&txf->tgi[i].y, tmp);
      }
    }
    txf->tgvi = (TexGlyphVertexInfo *)
      malloc(txf->num_glyphs * sizeof(TexGlyphVertexInfo));
    if (txf->tgvi == NULL) {
      lastError = "out of memory.";
      goto error;
    }
    w = txf->tex_width;
    h = txf->tex_height;
    txf->max_width = 0;
    xstep = 0.5 / w;
    ystep = 0.5 / h;
    for (i = 0; i < txf->num_glyphs; i++) {
      TexGlyphInfo *tgi;

      tgi = &txf->tgi[i];
      txf->tgvi[i].t0[0] = tgi->x / w - xstep; // MT - xstep
      txf->tgvi[i].t0[1] = tgi->y / h - ystep; // MT - ystep
      txf->tgvi[i].v0[0] = tgi->xoffset;
      txf->tgvi[i].v0[1] = tgi->yoffset;
      txf->tgvi[i].t1[0] = (tgi->x + tgi->width) / w + xstep;
      txf->tgvi[i].t1[1] = tgi->y / h - ystep; // MT - ystep
      txf->tgvi[i].v1[0] = tgi->xoffset + tgi->width;
      txf->tgvi[i].v1[1] = tgi->yoffset;
      txf->tgvi[i].t2[0] = (tgi->x + tgi->width) / w + xstep;
      txf->tgvi[i].t2[1] = (tgi->y + tgi->height) / h + ystep;
      txf->tgvi[i].v2[0] = tgi->xoffset + tgi->width;
      txf->tgvi[i].v2[1] = tgi->yoffset + tgi->height;
      txf->tgvi[i].t3[0] = tgi->x / w - xstep; // MT - xstep
      txf->tgvi[i].t3[1] = (tgi->y + tgi->height) / h + ystep;
      txf->tgvi[i].v3[0] = tgi->xoffset;
      txf->tgvi[i].v3[1] = tgi->yoffset + tgi->height;
      txf->tgvi[i].advance = tgi->advance;

      if(tgi->width > txf->max_width) txf->max_width = tgi->width;
    }

    min_glyph = txf->tgi[0].c;
    max_glyph = txf->tgi[0].c;
    for (i = 1; i < txf->num_glyphs; i++) {
      if (txf->tgi[i].c < min_glyph) {
	min_glyph = txf->tgi[i].c;
      }
      if (txf->tgi[i].c > max_glyph) {
	max_glyph = txf->tgi[i].c;
      }
    }
    txf->min_glyph = min_glyph;
    txf->range = max_glyph - min_glyph + 1;

    txf->lut = (TexGlyphVertexInfo **)
      calloc(txf->range, sizeof(TexGlyphVertexInfo *));
    if (txf->lut == NULL) {
      lastError = "out of memory.";
      goto error;
    }
    for (i = 0; i < txf->num_glyphs; i++) {
      txf->lut[txf->tgi[i].c - txf->min_glyph] = &txf->tgvi[i];
    }

    switch (format) {
    case TXF_FORMAT_BYTE:
      if (useLuminanceAlpha) {
	unsigned char *orig;

	orig = (unsigned char *) malloc(txf->tex_width * txf->tex_height);
	if (orig == NULL) {
	  lastError = "out of memory.";
	  goto error;
	}
	got = fread(orig, 1, txf->tex_width * txf->tex_height, file);
	EXPECT(txf->tex_width * txf->tex_height);
	txf->teximage = (unsigned char *)
	  malloc(2 * txf->tex_width * txf->tex_height);
	if (txf->teximage == NULL) {
	  lastError = "out of memory.";
	  goto error;
	}
	for (i = 0; i < txf->tex_width * txf->tex_height; i++) {
	  txf->teximage[i * 2] = orig[i];
	  txf->teximage[i * 2 + 1] = orig[i];
	}
	free(orig);
      } else {
	txf->teximage = (unsigned char *)
	  malloc(txf->tex_width * txf->tex_height);
	if (txf->teximage == NULL) {
	  lastError = "out of memory.";
	  goto error;
	}
	got = fread(txf->teximage, 1, txf->tex_width * txf->tex_height, file);
	EXPECT(txf->tex_width * txf->tex_height);
      }
      break;
    case TXF_FORMAT_BITMAP:
      width = txf->tex_width;
      height = txf->tex_height;
      stride = (width + 7) >> 3;
      texbitmap = (unsigned char *) malloc(stride * height);
      if (texbitmap == NULL) {
	lastError = "out of memory.";
	goto error;
      }
      got = fread(texbitmap, 1, stride * height, file);
      EXPECT(stride * height);
      if (useLuminanceAlpha) {
	txf->teximage = (unsigned char *) calloc(width * height * 2, 1);
	if (txf->teximage == NULL) {
	  lastError = "out of memory.";
	  goto error;
	}
	for (i = 0; i < height; i++) {
	  for (j = 0; j < width; j++) {
	    if (texbitmap[i * stride + (j >> 3)] & (1 << (j & 7))) {
	      txf->teximage[(i * width + j) * 2] = 255;
	      txf->teximage[(i * width + j) * 2 + 1] = 255;
	    }
	  }
	}
      } else {
	txf->teximage = (unsigned char *) calloc(width * height, 1);
	if (txf->teximage == NULL) {
	  lastError = "out of memory.";
	  goto error;
	}
	for (i = 0; i < height; i++) {
	  for (j = 0; j < width; j++) {
	    if (texbitmap[i * stride + (j >> 3)] & (1 << (j & 7))) {
	      txf->teximage[i * width + j] = 255;
	    }
	  }
	}
      }
      free(texbitmap);
      break;
    }

    fclose(file);
    return txf;

  error:

    if (txf) {
      if (txf->tgi)
	free(txf->tgi);
      if (txf->tgvi)
	free(txf->tgvi);
      if (txf->lut)
	free(txf->lut);
      if (txf->teximage)
	free(txf->teximage);
      free(txf);
    }
    if (file)
      fclose(file);
    return NULL;
  }

  /**************************************************************************/

  GLuint txfEstablishTexture(TexFont * txf, GLuint texobj,
			     GLboolean setupMipmaps)
  {
    if (txf->texobj == 0) {
      if (texobj == 0) {
	glGenTextures(1, &txf->texobj);
      } else {
	txf->texobj = texobj;
      }
    }
    glBindTexture(GL_TEXTURE_2D, txf->texobj);

    if (useLuminanceAlpha) {
      if (setupMipmaps) {
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE_ALPHA,
			  txf->tex_width, txf->tex_height,
			  GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, txf->teximage);
      } else {
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA,
		     txf->tex_width, txf->tex_height, 0,
		     GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, txf->teximage);
      }
    } else {
      if (setupMipmaps) {
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_INTENSITY4,
			  txf->tex_width, txf->tex_height,
			  GL_LUMINANCE, GL_UNSIGNED_BYTE, txf->teximage);
      } else {
	glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY4,
		     txf->tex_width, txf->tex_height, 0,
		     GL_LUMINANCE, GL_UNSIGNED_BYTE, txf->teximage);
      }
    }

    // MT: tried changing MIN/MAG filters ... bad idea.

    return txf->texobj;
  }

  /**************************************************************************/

  void txfBindFontTexture(TexFont * txf)
  {
    glBindTexture(GL_TEXTURE_2D, txf->texobj);
  }

  /**************************************************************************/

  void txfUnloadFont(TexFont * txf)
  {
    if (txf->texobj) {
      glDeleteTextures(1, &txf->texobj);
    }
    if (txf->teximage) {
      free(txf->teximage);
    }
    free(txf->tgi);
    free(txf->tgvi);
    free(txf->lut);
    free(txf);
  }

  /**************************************************************************/

  void txfGetStringMetrics(TexFont * txf, const char *string, int len,
			   int &width, int &max_ascent, int &max_descent)
  {
    TexGlyphVertexInfo *tgvi;
    int     w, i;
    int ma = 0, md = 0;

    w = 0;
    for (i = 0; i < len; i++) {
      if (string[i] == 27) {
	switch (string[i + 1]) {
	case 'M':
	  i += 4;
	  break;
	case 'T':
	  i += 7;
	  break;
	case 'L':
	  i += 7;
	  break;
	case 'F':
	  i += 13;
	  break;
	}
      } else {
	tgvi = getTCVI(txf, string[i]);
	w += int(tgvi->advance);
	ma = TMath::Max(ma, (int)( tgvi->v3[1]));
	md = TMath::Max(md, (int)(-tgvi->v0[1]));
      }
    }
    width = w;
    max_ascent  = ma; // txf->max_ascent;
    max_descent = md; // txf->max_descent;
    // printf("%d %d %d %d\n", txf->max_ascent, txf->max_descent, ma, md);

  }

  /**************************************************************************/

  void txfRenderGlyph(TexFont * txf, int c)
  {
    TexGlyphVertexInfo *tgvi;

    tgvi = getTCVI(txf, c);
    glBegin(GL_QUADS);
    glTexCoord2fv(tgvi->t0);
    glVertex2sv(tgvi->v0);
    glTexCoord2fv(tgvi->t1);
    glVertex2sv(tgvi->v1);
    glTexCoord2fv(tgvi->t2);
    glVertex2sv(tgvi->v2);
    glTexCoord2fv(tgvi->t3);
    glVertex2sv(tgvi->v3);
    glEnd();
    glTranslatef(tgvi->advance, 0.0, 0.0);
  }

  void txfRenderString(TexFont * txf, const char *string, int len,
		       bool keep_pos)
  {
    int i;
    if(keep_pos) glPushMatrix();
    for (i = 0; i < len; i++) {
      txfRenderGlyph(txf, string[i]);
    }
    if(keep_pos) glPopMatrix();
  }

  void txfRenderString(TexFont * txf, const char *string, int len,
		       GLfloat maxx, GLfloat fadew,
		       bool keep_pos)
  {
    GLfloat x = 0, xg0, xg1, yg0, yg1, f0, f1;
    fadew *= txf->max_width;
    GLfloat xfade = maxx - fadew;

    GLfloat col[4];
    glGetFloatv(GL_CURRENT_COLOR, col);

    glBegin(GL_QUADS);
    for (int i = 0; i < len; i++) {

      TexGlyphVertexInfo *tgvi;

      tgvi = getTCVI(txf, string[i]);

      xg0 = x + tgvi->v0[0];
      xg1 = x + tgvi->v1[0];
      yg0 = tgvi->v0[1];
      yg1 = tgvi->v2[1];

      if(xg1 > xfade) {
	f0 = 1;	if(xg0 > xfade) f0 *= 1 - (xg0-xfade)/fadew;
	f1 = 1 - (xg1-xfade)/fadew;

	// printf("XX %s %c %f %f x(%f,%f) y(%f,%f)\n",
	//        string, string[i], f0, f1,
	//        xg0, xg1,yg0, yg1);

	glColor4f(f0*col[0], f0*col[1], f0*col[2], f0*col[3]);
	glTexCoord2fv(tgvi->t0);    glVertex2f(xg0, yg0);
	glColor4f(f1*col[0], f1*col[1], f1*col[2], f1*col[3]);
	glTexCoord2fv(tgvi->t1);    glVertex2f(xg1, yg0);
	glTexCoord2fv(tgvi->t2);    glVertex2f(xg1, yg1);
	glColor4f(f0*col[0], f0*col[1], f0*col[2], f0*col[3]);
	glTexCoord2fv(tgvi->t3);    glVertex2f(xg0, yg1);
      } else {
	glTexCoord2fv(tgvi->t0);    glVertex2f(xg0, yg0);
	glTexCoord2fv(tgvi->t1);    glVertex2f(xg1, yg0);
	glTexCoord2fv(tgvi->t2);    glVertex2f(xg1, yg1);
	glTexCoord2fv(tgvi->t3);    glVertex2f(xg0, yg1);
      }

      x += tgvi->advance;
      if(x > maxx) break;
    }
    glEnd();

    if(!keep_pos) glTranslatef(x, 0.0, 0.0);
  }

  /**************************************************************************/

  void txfRenderGlyphZW(TexFont * txf, int c, float z, float w)
  {
    TexGlyphVertexInfo *tgvi;

    tgvi = getTCVI(txf, c);
    glBegin(GL_QUADS);
    glTexCoord2fv(tgvi->t0);
    glVertex4f(tgvi->v0[0], tgvi->v0[1], z, w);
    glTexCoord2fv(tgvi->t1);
    glVertex4f(tgvi->v1[0], tgvi->v1[1], z, w);
    glTexCoord2fv(tgvi->t2);
    glVertex4f(tgvi->v2[0], tgvi->v2[1], z, w);
    glTexCoord2fv(tgvi->t3);
    glVertex4f(tgvi->v3[0], tgvi->v3[1], z, w);
    glEnd();
    glTranslatef(tgvi->advance, 0.0, 0.0);
  }

  void txfRenderStringZW(TexFont * txf, const char *string, int len,
			 float z, float w, bool keep_pos)
  {
    int i;

    if(keep_pos) glPushMatrix();
    for (i = 0; i < len; i++) {
      txfRenderGlyphZW(txf, string[i], z, w);
    }
    if(keep_pos) glPopMatrix();
  }

  /**************************************************************************/

  enum {
    MONO, TOP_BOTTOM, LEFT_RIGHT, FOUR
  };

  /**************************************************************************/

  void txfRenderFancyString(TexFont * txf, char *string, int len)
  {
    TexGlyphVertexInfo *tgvi;
    GLubyte c[4][3];
    int mode = MONO;
    int i;

    for (i = 0; i < len; i++) {
      if (string[i] == 27) {
	switch (string[i + 1]) {
	case 'M':
	  mode = MONO;
	  glColor3ubv((GLubyte *) & string[i + 2]);
	  i += 4;
	  break;
	case 'T':
	  mode = TOP_BOTTOM;
	  memcpy(c, &string[i + 2], 6);
	  i += 7;
	  break;
	case 'L':
	  mode = LEFT_RIGHT;
	  memcpy(c, &string[i + 2], 6);
	  i += 7;
	  break;
	case 'F':
	  mode = FOUR;
	  memcpy(c, &string[i + 2], 12);
	  i += 13;
	  break;
	}
      } else {
	switch (mode) {
	case MONO:
	  txfRenderGlyph(txf, string[i]);
	  break;
	case TOP_BOTTOM:
	  tgvi = getTCVI(txf, string[i]);
	  glBegin(GL_QUADS);
	  glColor3ubv(c[0]);
	  glTexCoord2fv(tgvi->t0);
	  glVertex2sv(tgvi->v0);
	  glTexCoord2fv(tgvi->t1);
	  glVertex2sv(tgvi->v1);
	  glColor3ubv(c[1]);
	  glTexCoord2fv(tgvi->t2);
	  glVertex2sv(tgvi->v2);
	  glTexCoord2fv(tgvi->t3);
	  glVertex2sv(tgvi->v3);
	  glEnd();
	  glTranslatef(tgvi->advance, 0.0, 0.0);
	  break;
	case LEFT_RIGHT:
	  tgvi = getTCVI(txf, string[i]);
	  glBegin(GL_QUADS);
	  glColor3ubv(c[0]);
	  glTexCoord2fv(tgvi->t0);
	  glVertex2sv(tgvi->v0);
	  glColor3ubv(c[1]);
	  glTexCoord2fv(tgvi->t1);
	  glVertex2sv(tgvi->v1);
	  glColor3ubv(c[1]);
	  glTexCoord2fv(tgvi->t2);
	  glVertex2sv(tgvi->v2);
	  glColor3ubv(c[0]);
	  glTexCoord2fv(tgvi->t3);
	  glVertex2sv(tgvi->v3);
	  glEnd();
	  glTranslatef(tgvi->advance, 0.0, 0.0);
	  break;
	case FOUR:
	  tgvi = getTCVI(txf, string[i]);
	  glBegin(GL_QUADS);
	  glColor3ubv(c[0]);
	  glTexCoord2fv(tgvi->t0);
	  glVertex2sv(tgvi->v0);
	  glColor3ubv(c[1]);
	  glTexCoord2fv(tgvi->t1);
	  glVertex2sv(tgvi->v1);
	  glColor3ubv(c[2]);
	  glTexCoord2fv(tgvi->t2);
	  glVertex2sv(tgvi->v2);
	  glColor3ubv(c[3]);
	  glTexCoord2fv(tgvi->t3);
	  glVertex2sv(tgvi->v3);
	  glEnd();
	  glTranslatef(tgvi->advance, 0.0, 0.0);
	  break;
	}
      }
    }
  }

  /**************************************************************************/

  int txfInFont(TexFont * txf, int c)
  {
    /* NOTE: No uppercase/lowercase substituion. */
    if ((c >= txf->min_glyph) && (c < txf->min_glyph + txf->range)) {
      if (txf->lut[c - txf->min_glyph]) {
	return 1;
      }
    }
    return 0;
  }

  /**************************************************************************/

}

/**************************************************************************/
// MT crap starts here.
// Need a proper implementation of all this ... but so little time ...
/**************************************************************************/

GLTextNS::TextLineData::TextLineData(TexFont *txf, TString line) : text(line)
{
  txfGetStringMetrics(txf, text.Data(), text.Length(), width, ascent, descent);
  hfull   = ascent + descent;
}

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
  TexFont *txf = font_rnr->GetFont();

  lStr_t lines;
  lTLD_t tlds;
  int
    max_width = 0,
    width     = 0,
    ascent    = txf->max_ascent,
    descent   = txf->max_descent,
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
  txfBindFontTexture(txf);

  glTranslatef(bs.lm, -(bs.tm + descent + ascent), 0);
  for(lTLD_i l=tlds.begin(); l!=tlds.end(); ++l) {
    glPushMatrix();
    if(l->width != max_width) {
      switch(bs.align) {
      case 'c': case 'C': glTranslatef((max_width - l->width)/2, 0, 0); break;
      case 'r': case 'R': glTranslatef((max_width - l->width),   0, 0); break;
      }
    }
    txfRenderString(txf, l->text.Data(), l->text.Length(), false);
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
  TexFont *txf = font_rnr->GetFont();
  RNRDRIVER_GET_RNRMOD_LENS(nrc, rd, ZRlNodeMarkup);

  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT);

  glNormal3f(0, 0, 1);

  glPolygonMode(GL_FRONT, GL_FILL);
  glDisable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  int width, ascent, descent;
  GLTextNS::txfGetStringMetrics(txf, text.Data(), text.Length(),
				width, ascent, descent);
  ascent  = txf->max_ascent;
  descent = txf->max_descent;

  int   h_box = ascent + descent;
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
  GLTextNS::txfBindFontTexture(txf);
  GLTextNS::txfRenderString(txf, text.Data(), text.Length());
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
  TexFont *txf = font_rnr->GetFont();

  glPushAttrib(GL_TEXTURE_BIT      |
	       GL_LIGHTING_BIT     |
	       GL_COLOR_BUFFER_BIT |
	       GL_POLYGON_BIT);


  glPolygonMode(GL_FRONT, GL_FILL);
  glDisable(GL_LIGHTING);
  glDisable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  int width, ascent, descent;
  GLTextNS::txfGetStringMetrics(txf, text.Data(), text.Length(),
                                width, ascent, descent);
  ascent  = txf->max_ascent;
  descent = txf->max_descent;
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

  GLTextNS::txfBindFontTexture(txf);
  GLTextNS::txfRenderString(txf, text.Data(), text.Length());

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
  // z    - z coordinate in depth-buffer coordinates (0 -> 1)
  // If front_col == 0 text is drawn in white.
  // If back_col  != 0 renders a rectangle of that color behind the text.

  RNRDRIVER_GET_RNRMOD_BOTH(font, rd, ZRlFont);
  TexFont *txf = font_rnr->GetFont();

  glPushAttrib(GL_TEXTURE_BIT      |
	       GL_LIGHTING_BIT     |
	       GL_COLOR_BUFFER_BIT |
	       GL_POLYGON_BIT);

  glPolygonMode(GL_FRONT, GL_FILL);
  glDisable(GL_LIGHTING);
  glDisable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  int width, ascent, descent;
  GLTextNS::txfGetStringMetrics(txf, text.Data(), text.Length(),
                                width, ascent, descent);
  ascent  = txf->max_ascent;
  descent = txf->max_descent;
  int   h_box = ascent + descent;
  float scale = float(font_lens->GetSize()) / ascent;

  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadMatrixd(rd->GetProjBase()->Array());
  glOrtho(0, rd->GetWidth(), 0, rd->GetHeight(), 0, -1);

  if(x < 0)
    x = rd->GetWidth() - TMath::Nint(width*scale) + x;
  glTranslatef(x, rd->GetHeight() - (yrow*h_box + ascent)*scale, z);
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

  GLTextNS::txfBindFontTexture(txf);
  GLTextNS::txfRenderString(txf, text.Data(), text.Length());

  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glPopAttrib();
}
