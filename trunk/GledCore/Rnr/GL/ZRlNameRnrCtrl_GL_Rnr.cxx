// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZRlNameRnrCtrl_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <TSystem.h>
#include <FL/gl.h>

/**************************************************************************/

void ZRlNameRnrCtrl_GL_Rnr::_init()
{
  mFont = 0;
}

/**************************************************************************/

// void ZRlNameRnrCtrl_GL_Rnr::PreDraw(RnrDriver* rd) {}

void ZRlNameRnrCtrl_GL_Rnr::Draw(RnrDriver* rd)
{
  ZRlNameRnrCtrl& x = *mZRlNameRnrCtrl;

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

  switch(x.mNameRnrOp) {
  case ZGlStateBase::GSO_On:
    if(mFont) {
      rd->fTexFont = mFont;
    }
    rd->SetRnrNames(true);
    rd->SetRnrTiles(x.GetRnrTiles());
    rd->SetRnrFrames(x.GetRnrFrames());
    rd->SetNameOffset(x.GetNameOffset());
    rd->SetTextSize(x.GetTextSize());
    rd->RefTextCol() = x.GetTextCol();
    rd->RefTileCol() = x.GetTileCol();
    rd->SetTilePos(x.GetTilePos());
    break;
  case ZGlStateBase::GSO_Off:
    rd->SetRnrNames(false);
    break;
  }
}

// void ZRlNameRnrCtrl_GL_Rnr::PostDraw(RnrDriver* rd) {}
