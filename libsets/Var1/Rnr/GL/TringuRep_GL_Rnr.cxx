// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TringuRep_GL_Rnr.h"
#include <Glasses/Tringula.h>
#include <Glasses/TriMesh.h>
#include <RnrBase/RnrDriver.h>
#include <Rnr/GL/TringTvor_GL_Rnr.h>

#include <GL/glew.h>

#define PARENT ZNode_GL_Rnr

//==============================================================================

void TringuRep_GL_Rnr::_init()
{}

TringuRep_GL_Rnr::TringuRep_GL_Rnr(TringuRep* idol) :
  ZNode_GL_Rnr(idol),
  mTringuRep(idol)
{
  _init();
}

TringuRep_GL_Rnr::~TringuRep_GL_Rnr()
{}

//==============================================================================

//void TringuRep_GL_Rnr::PreDraw(RnrDriver* rd) {}

void TringuRep_GL_Rnr::Draw(RnrDriver* rd)
{
  TringuRep &TR = * mTringuRep;

  {
    GL_Capability_Switch light(GL_LIGHTING, *TR.mLightField == 0);
    GL_Enum_Holder       shade(GL_SHADE_MODEL, TR.bSmoothShading ? GL_SMOOTH : GL_FLAT, glShadeModel);

    PARENT::Draw(rd);
  }

  // Brutally descend Tringula's extendios.
  // Could use render scheme.
  // Even better, could be a bit smart about what needs to be drawn.
  // Results of the last collision detection pass could be used.
  Tringula  &T = * TR.GetTringula();
  rd->Render(rd->GetLensRnr(*T.mStatos));
  rd->Render(rd->GetLensRnr(*T.mDynos));
  rd->Render(rd->GetLensRnr(*T.mFlyers));
  rd->Render(rd->GetLensRnr(*T.mLandMarks));
}

//void TringuRep_GL_Rnr::PostDraw(RnrDriver* rd) {}

void TringuRep_GL_Rnr::Render(RnrDriver* rd)
{
  TringuRep &TR = * mTringuRep;
  Tringula  &T  = * TR.GetTringula();

  TringTvor_GL_Rnr rnr(T.GetMesh()->GetTTvor());

  rnr.BeginRender(TR.bSmoothShading, TR.bTringStrips);

  if (TR.bSmoothShading)
    rnr.SetColorArray(TR.GetVertexColorArray());
  else
    rnr.SetColorArray(TR.GetTriangleColorArray());

  // If we ever use textures -- shaders would make more sense.
  // rnr.SetTextureArray(...);

  rnr.Render();

  rnr.EndRender();
}
