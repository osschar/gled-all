// $Id$

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
#include <GL/glew.h>

class RnrDriver;
class ZColor;

class FTFont;

namespace GLTextNS
{
  struct BoxSpecs
  {
    int     lm, rm, tm, bm;
    int     lineskip;
    char    align;
    TString pos;

    void _init() { align = 'l'; lineskip = 0; }

    BoxSpecs()
    { lm = rm = 3; tm = 0; bm = 2; _init(); }

    BoxSpecs(int lr, int tb)
    { lm = rm = lr; tm = bm = tb; _init(); }

    BoxSpecs(int l, int r, int t, int b)
    { lm = l; rm = r; tm = t; bm = b; _init(); }
  };

  struct TextLineData
  {
    float   width, ascent, descent, hfull;
    TString text;

    TextLineData(FTFont *ftf, const TString& line);
  };

  extern Float_t MeasureWidth(FTFont *ftf, const TString& txt);
  extern Float_t MeasureWidth(FTFont *ftf, const TString& txt,
			      Float_t& ascent, Float_t& descent);

  extern void RnrTextBar(RnrDriver* rd, const TString& text);

  extern void RnrTextBar(RnrDriver* rd, const TString& text,
			 BoxSpecs& bs, float zoffset=0);

  extern void RnrTextPoly(RnrDriver* rd, const TString& text);

  extern void RnrText(RnrDriver* rd, const TString& text,
		      int x, int y, float z,
		      const ZColor* front_col, const ZColor* back_col=0);

  extern void RnrTextAt(RnrDriver* rd, const TString& text,
			int x, int yrow, float z,
			const ZColor* front_col, const ZColor* back_col=0);
}

#endif
