// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_WGlFrameStyle_GL_RNR_H
#define GledCore_WGlFrameStyle_GL_RNR_H

#include <Glasses/WGlFrameStyle.h>
#include <Rnr/GL/ZRnrModBase_GL_Rnr.h>

class FTFont;

class WGlFrameStyle_GL_Rnr : public ZRnrModBase_GL_Rnr
{
 private:
  void _init();
  void render_triangle(int id, float dx, float dy, const ZColor* col);

 protected:
  WGlFrameStyle*	mWGlFrameStyle;

 public:
  WGlFrameStyle_GL_Rnr(WGlFrameStyle* idol) :
    ZRnrModBase_GL_Rnr(idol), mWGlFrameStyle(idol)
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

  void BoxLimits(float& dx, float& dy,
		 float& x, float& y, float& w, float& h);
  void StudyText(FTFont *txf, const TString& label,
		 float& scale,
		 float& x, float& y, float& w, float& h);


  void RenderTile (float dx, float dy, bool belowmouse=false,
		   const ZColor* col=0);
  void RenderFrame(float dx, float dy);
  void RenderText (FTFont *txf, const TString& label,
		   float x, float y, float w, float h);

  void FullRender(FTFont *txf, const TString& label,
		  float dx, float dy, bool belowmouse=false);

  void FullSymbolRender(FTFont *txf, const TString& label,
			float dx, float dy, bool belowmouse=false,
			const ZColor* col1=0, const ZColor* col2=0);

}; // endclass WGlFrameStyle_GL_Rnr

#endif
