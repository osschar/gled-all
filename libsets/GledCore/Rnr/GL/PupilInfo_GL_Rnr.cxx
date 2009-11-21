// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "PupilInfo_GL_Rnr.h"
#include "ZRlFont_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>
#include <Rnr/GL/GLTextNS.h>
#include <GL/glew.h>

#include <Gled/Gled.h>

namespace
{
  ZRlFont*        def_font = 0;
  ZRlFont_GL_Rnr* def_font_rnr = 0;
  RnrMod          font_rnrmod;
}

void PupilInfo_GL_Rnr::_init()
{
  static const Exc_t _eh("PupilInfo_GL_Rnr::_init ");

  if(def_font == 0) {
    def_font = new ZRlFont();
    def_font->SetFontFile
      (GForm("%s/fontdefault.txf", Gled::theOne->GetLibDir()));
    def_font_rnr = new ZRlFont_GL_Rnr(def_font);
    if(def_font_rnr->LoadFont() == false) {
      throw(_eh + "default font not found.");
    }
    font_rnrmod.fLens = def_font;
    font_rnrmod.fRnr  = def_font_rnr;
  }
}

//==============================================================================

void PupilInfo_GL_Rnr::InitRendering(RnrDriver* rd)
{
  // Virtual method called before rendering starts.
  // GL is already initialized and camera is set-up.
  // Thus this is a good place to extract values from RnrDriver.

  // Extract camera params and copy them to the other side.

  PupilInfo& PI = * mPupilInfo;
  PI.mRnrCamFix  = rd->GetCamFixTrans();
  PI.mRnrScreenW = rd->GetWidth();
  PI.mRnrScreenH = rd->GetHeight();
}

//==============================================================================

int PupilInfo_GL_Rnr::Handle(RnrDriver* rd, Fl_Event& ev)
{
  // Handle event -- just copy some data over to the other side.
  // Always return 0 so that the event will be processed further.

  PupilInfo& PI = * mPupilInfo;

  PI.mMouseX      = rd->GL()->GetMouseX();
  PI.mMouseY      = rd->GL()->GetMouseY();
  PI.mMouseRayPos = rd->GL()->RefMouseRayPos();
  PI.mMouseRayDir = rd->GL()->RefMouseRayDir();

  return 0;
}

//==============================================================================

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
