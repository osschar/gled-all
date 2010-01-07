// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TringuRep_GL_Rnr.h"
#include <Glasses/Extendio.h>
#include <Glasses/TSPupilInfo.h>
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

  // Brutally descend into Tringula's extendios and tubes.
  // Could use render scheme.
  // Even better, could be a bit smart about what needs to be drawn.
  // Results of the last collision detection pass could be used.
  Tringula  &T = * TR.GetTringula();
  rd->Render(rd->GetLensRnr(*T.mStatos));
  rd->Render(rd->GetLensRnr(*T.mDynos));
  rd->Render(rd->GetLensRnr(*T.mFlyers));
  rd->Render(rd->GetLensRnr(*T.mLandMarks));
  rd->Render(rd->GetLensRnr(*T.mTubes));

  // Render selected Extendios.
  TSPupilInfo &PI = * TR.GetPupilInfo();
  ZList *sel = PI.GetSelection();
  if (sel && ! sel->IsEmpty())
  {
    GL_Capability_Switch ligt_off(GL_LIGHTING, false);
    GL_Float_Holder      fat_line(GL_LINE_WIDTH, 2, glLineWidth);
    glColor3fv(PI.PtrSelectionColor()->array());

    AList::Stepper<> stepper(sel);
    while (stepper.step())
    {
      Extendio* ext = (Extendio*) *stepper;
      glPushMatrix();
      glMultMatrixf(ext->RefLastTrans().Array());
      TringTvor_GL_Rnr::RenderCEBBox(ext->GetMesh()->GetTTvor()->mCtrExtBox, 1.01f, false);
      glPopMatrix();
    }
  }
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
