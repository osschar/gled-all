// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZRlFont_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <FL/gl.h>

/**************************************************************************/

void ZRlFont_GL_Rnr::_init()
{
  mFont = 0;
  LoadFont();
}

/**************************************************************************/

void ZRlFont_GL_Rnr::AbsorbRay(Ray& ray)
{
  if(ray.fFID == ZRlFont::FID() && ray.fRQN == ZRlFont::PRQN_font_change) {
    LoadFont();
  }
}

/**************************************************************************/

void ZRlFont_GL_Rnr::PreDraw(RnrDriver* rd)
{
  if(mFont) {
    rd->PushRnrMod(ZRlFont::FID(), mZRlFont);
    mExFont = rd->GetTexFont();
    rd->SetTexFont(mFont);
  }
}

void ZRlFont_GL_Rnr::Draw(RnrDriver* rd)
{
  if(mFont) {
    rd->SetDefRnrMod(ZRlFont::FID(), mZRlFont);
    rd->SetTexFont(mFont);
  }
}

void ZRlFont_GL_Rnr::PostDraw(RnrDriver* rd)
{
  if(mFont) {
    rd->PopRnrMod(ZRlFont::FID());
    rd->SetTexFont(mExFont);
  }
}

/**************************************************************************/

bool ZRlFont_GL_Rnr::LoadFont()
{
  static const string _eh("ZRlFont_GL_Rnr::LoadFont ");

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

/**************************************************************************/

void ZRlFont_GL_Rnr::MakeDefault(RnrDriver* rd)
{
  rd->SetDefRnrMod(ZRlFont::FID(), mZRlFont);
  rd->SetTexFont(mFont);
}
