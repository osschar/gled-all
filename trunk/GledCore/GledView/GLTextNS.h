// $Header$

#ifndef GledCore_GLTextNS_H
#define GledCore_GLTextNS_H

// The following implementation is based on TexFont API,
// implementation and accompanying programs by Mark J. Kilgard.
// Original license:

/* Copyright (c) Mark J. Kilgard, 1997. */
/* This program is freely distributable without licensing fees  and is
   provided without guarantee or warrantee expressed or implied. This
   program is -not- in the public domain. */

#include <Gled/GledTypes.h>
#include <GL/gl.h>

class RnrDriver;
class ZColor;

namespace GLTextNS {

#define TXF_FORMAT_BYTE		0
#define TXF_FORMAT_BITMAP	1

  struct TexGlyphInfo {
    unsigned short c;       /* Potentially support 16-bit glyphs. */
    unsigned char width;
    unsigned char height;
    signed char xoffset;
    signed char yoffset;
    signed char advance;
    char dummy;           /* Space holder for alignment reasons. */
    short x;
    short y;
  };

  struct TexGlyphVertexInfo {
    GLfloat t0[2];
    GLshort v0[2];
    GLfloat t1[2];
    GLshort v1[2];
    GLfloat t2[2];
    GLshort v2[2];
    GLfloat t3[2];
    GLshort v3[2];
    GLfloat advance;
  };

  struct TexFont {
    GLuint texobj;
    int tex_width;
    int tex_height;
    int max_ascent;
    int max_descent;
    int num_glyphs;
    int min_glyph;
    int range;
    unsigned char *teximage;
    TexGlyphInfo *tgi;
    TexGlyphVertexInfo *tgvi;
    TexGlyphVertexInfo **lut;
  };

  extern char *txfErrorString(void);

  extern TexFont *txfLoadFont(const char *filename);

  extern void txfUnloadFont(TexFont* txf);

  extern GLuint txfEstablishTexture(TexFont* txf, GLuint texobj,
				    GLboolean setupMipmaps);

  extern void txfBindFontTexture(TexFont* txf);

  extern void txfGetStringMetrics(TexFont* txf, const char *string, int len,
				  int &width, int &max_ascent, int &max_descent);

  extern void txfRenderGlyph(TexFont* txf, int c);
  extern void txfRenderString(TexFont* txf, const char *string, int len,
			      bool keep_pos=true);

  extern void txfRenderGlyphZW(TexFont* txf, int c, float z, float w);
  extern void txfRenderStringZW(TexFont* txf, const char *string, int len,
				float z, float w, bool keep_pos=true);

  extern void txfRenderFancyString(TexFont* txf, char *string, int len);


  /**************************************************************************/
  // Here starts MT higher-level interface
  /**************************************************************************/

  struct BoxSpecs {
    int    lm, rm, tm, bm;
    int    lineskip;
    char   align;
    string pos;

    void _init() { align = 'l'; lineskip = 0; }

    BoxSpecs()
    { lm = rm = 3; tm = 0; bm = 2; _init(); }

    BoxSpecs(int lr, int tb)
    { lm = rm = lr; tm = bm = tb; _init(); }

    BoxSpecs(int l, int r, int t, int b)
    { lm = l; rm = r; tm = t; bm = b; _init(); }
  };

  struct TextLineData {
    int    width, ascent, descent, hfull;
    string text;

    TextLineData(TexFont *txf, string line);
  };

  extern void RnrTextBar(RnrDriver* rd, const string& text,
			 BoxSpecs& bs, float zoffset=0);

  extern void RnrTextPoly(RnrDriver* rd, const string& text);

  extern void RnrTextAt(RnrDriver* rd, const string& text,
			int x, int yrow, float z,
			const ZColor* front_col, const ZColor* back_col=0);
}

#endif
