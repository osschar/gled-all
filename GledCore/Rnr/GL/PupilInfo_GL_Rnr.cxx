// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "PupilInfo_GL_Rnr.h"
#include "ZRlFont_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <GledView/GLTextNS.h>
#include <FL/gl.h>

#include <TSystem.h>

namespace {
  ZRlFont*        def_font = 0;
  ZRlFont_GL_Rnr* def_font_rnr = 0;
}

void PupilInfo_GL_Rnr::_init()
{
  static const string _eh("PupilInfo_GL_Rnr::_init ");

  if(def_font == 0) {
    def_font = new ZRlFont();
    def_font->SetFontFile
      (GForm("%s/lib/fontdefault.txf", gSystem->Getenv("GLEDSYS")));
    def_font_rnr = new ZRlFont_GL_Rnr(def_font);
  }
  def_font_rnr->LoadFont();
}

/**************************************************************************/

void PupilInfo_GL_Rnr::PreDraw(RnrDriver* rd)
{
  // Default font
  def_font_rnr->MakeDefault(rd);

  // Default name-rendering on/off
  rd->SetRnrNames(mPupilInfo->GetRnrNames());
}

void PupilInfo_GL_Rnr::Draw(RnrDriver* rd)
{}

void PupilInfo_GL_Rnr::PostDraw(RnrDriver* rd)
{
  rd->CleanUpRnrModDefaults();
}
