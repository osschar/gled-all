// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "PupilInfo_GL_Rnr.h"
#include "ZRlFont_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>
#include <Rnr/GL/GLTextNS.h>
#include <GL/gl.h>

#include <TSystem.h>

namespace {
  ZRlFont*        def_font = 0;
  ZRlFont_GL_Rnr* def_font_rnr = 0;
  RnrMod          font_rnrmod;
}

void PupilInfo_GL_Rnr::_init()
{
  static const string _eh("PupilInfo_GL_Rnr::_init ");

  if(def_font == 0) {
    def_font = new ZRlFont();
    def_font->SetFontFile
      (GForm("%s/lib/fontdefault.txf", gSystem->Getenv("GLEDSYS")));
    def_font_rnr = new ZRlFont_GL_Rnr(def_font);
    def_font_rnr->LoadFont();
    font_rnrmod.fLens = def_font;
    font_rnrmod.fRnr  = def_font_rnr;
  }
}

/**************************************************************************/

void PupilInfo_GL_Rnr::PreDraw(RnrDriver* rd)
{
  rd->CleanUpRnrModDefaults();
  // Default font
  rd->SetDefRnrMod(ZRlFont::FID(), &font_rnrmod);
  // Default node markup: name-rendering on/off
  rd->GL()->SetMarkupNodes(mPupilInfo->GetRnrNames());
}

void PupilInfo_GL_Rnr::Draw(RnrDriver* rd)
{}

void PupilInfo_GL_Rnr::PostDraw(RnrDriver* rd)
{
  // Reset def font for overlay rendering.
  rd->SetDefRnrMod(ZRlFont::FID(), &font_rnrmod);
}
