// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZRlFont_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <FL/gl.h>

#include <TSystem.h>

/**************************************************************************/

void ZRlFont_GL_Rnr::_init()
{
  mFont = 0; mFontBuf = 0;
}

/**************************************************************************/

void ZRlFont_GL_Rnr::BindFont(RnrDriver* rd)
{
  ZRlFont& x = *mZRlFont;

  if(!x.mFontName.IsNull() && mFont == 0) {
    const char* file = GForm("%s/lib/font%s.txf",
			     gSystem->Getenv("GLEDSYS"), x.mFontName.Data());
    mFont = GLTextNS::txfLoadFont(file);
    if(mFont != 0) {
      GLTextNS::txfEstablishTexture(mFont, 0, GL_TRUE);
    } else {
      fprintf(stderr, "Problem loading font from file %s ; error: %s.\n",
              file, GLTextNS::txfErrorString());
    }
  }
  if(mFont) {
    rd->fTexFont = mFont;
    GLTextNS::txfBindFontTexture(mFont);
  }
}

/**************************************************************************/

void ZRlFont_GL_Rnr::PreDraw(RnrDriver* rd)
{
  mFontBuf = rd->fTexFont;
  BindFont(rd);
}

void ZRlFont_GL_Rnr::Draw(RnrDriver* rd)
{
  BindFont(rd);
}

void ZRlFont_GL_Rnr::PostDraw(RnrDriver* rd)
{
  rd->fTexFont = mFontBuf;
}
