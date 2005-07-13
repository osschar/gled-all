// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_WGlFrameStyle_GL_RNR_H
#define GledCore_WGlFrameStyle_GL_RNR_H

#include <Glasses/WGlFrameStyle.h>
#include <Rnr/GL/ZRnrModBase_GL_Rnr.h>

namespace GLTextNS { class TexFont; }

class WGlFrameStyle_GL_Rnr : public ZRnrModBase_GL_Rnr {
private:
  void _init();

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
  void StudyText(GLTextNS::TexFont *txf, string& label,
		 float& scale,
		 float& x, float& y, float& w, float& h);
		  

  void RenderTile (float dx, float dy, bool belowmouse=false);
  void RenderFrame(float dx, float dy);
  void RenderText (GLTextNS::TexFont *txf, string& label,
		   float x, float y, float w, float h);

  void FullRender(GLTextNS::TexFont *txf, string& label,
		  float dx, float dy, bool belowmouse=false);

}; // endclass WGlFrameStyle_GL_Rnr

#endif
