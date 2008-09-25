// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZRlFont_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <GL/gl.h>

#define PARENT ZRnrModBase_GL_Rnr

/**************************************************************************/

void ZRlFont_GL_Rnr::_init()
{
  mFont = 0;
  LoadFont();
}

/**************************************************************************/

void ZRlFont_GL_Rnr::AbsorbRay(Ray& ray)
{
  static const Exc_t _eh("ZRlFont_GL_Rnr::AbsorbRay ");

  if (ray.fFID == ZRlFont::FID() && ray.fRQN == ZRlFont::PRQN_font_change)
  {
    if (LoadFont() == false)
      throw(_eh + "loading of font '" + mZRlFont->mFontFile + "' failed.");
  }
}

/**************************************************************************/

void ZRlFont_GL_Rnr::PreDraw(RnrDriver* rd)
{
  if(mFont) {
    PARENT::PreDraw(rd);
    update_tring_stamp(rd);
    rd->PushRnrMod(ZRlFont::FID(), mRnrMod);
  }
}

void ZRlFont_GL_Rnr::Draw(RnrDriver* rd)
{
  if(mFont) {
    update_tring_stamp(rd);
    rd->SetDefRnrMod(ZRlFont::FID(), mRnrMod);
  }
}

void ZRlFont_GL_Rnr::PostDraw(RnrDriver* rd)
{
  if(mFont) {
    rd->PopRnrMod(ZRlFont::FID());
    PARENT::PostDraw(rd);
  }
}

/**************************************************************************/

bool ZRlFont_GL_Rnr::LoadFont()
{
  static const Exc_t _eh("ZRlFont_GL_Rnr::LoadFont ");

  if(mFont) {
    GLTextNS::txfUnloadFont(mFont);
    mFont = 0;
  }
  ZRlFont& x = *mZRlFont;
  if(!x.mFontFile.IsNull()) {
    mFont = GLTextNS::txfLoadFont(x.mFontFile.Data());
    if(mFont != 0) {
      GLTextNS::txfEstablishTexture(mFont, 0, GL_TRUE);
      return true;
    } else {
      ISerr(_eh + GForm("Error loading font from file '%s': %s",
			x.mFontFile.Data(), GLTextNS::txfErrorString()));
    }
  }
  return false;
}
