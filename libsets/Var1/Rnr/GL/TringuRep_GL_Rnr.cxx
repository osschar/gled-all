// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TringuRep_GL_Rnr.h"
#include <Glasses/Extendio.h>
#include "Glasses/ExtendioSpiritio.h"
#include <Glasses/ExtendioExplosion.h>
#include <Glasses/TSPupilInfo.h>
#include <Glasses/Tringula.h>
#include <Glasses/TriMesh.h>
#include <RnrBase/RnrDriver.h>
#include <Rnr/GL/TringTvor_GL_Rnr.h>

#include <Eye/Eye.h>

#include <Glasses/AlBuffer.h>
#include <Glasses/AlSource.h>
#include <Glasses/ZQueen.h>
#include <Glasses/ZHashList.h>

#include <GL/glew.h>


#define PARENT ZNode_GL_Rnr

//==============================================================================

TringuRep_GL_Rnr::TringuRep_GL_Rnr(TringuRep* idol) :
  ZNode_GL_Rnr(idol),
  mTringuRep(idol),
  mTringulaSpy(0),
  mSoundDir(0)
{}

TringuRep_GL_Rnr::~TringuRep_GL_Rnr()
{
  delete mTringulaSpy;
}

void TringuRep_GL_Rnr::SetImg(OptoStructs::ZGlassImg* newimg)
{
  PARENT::SetImg(newimg);
  mTringulaSpy = new TringulaSpy(fImg->fEye->DemanglePtr(mTringuRep->GetTringula()), this); 
  mSoundDir = dynamic_cast<AList*>(mTringuRep->GetQueen()->FindLensByPath("var/sounds"));
}

// In principle also need AbsorbRay() to check if Tringula link changes.
// This should not really happen, but then again, it might.
// Then also check for null tringula link.


AlBuffer* TringuRep_GL_Rnr::find_sound(const TString& effect)
{
  return (AlBuffer*) mSoundDir->GetElementByName(effect);
}

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

  {
    GL_Capability_Switch ligt_off(GL_LIGHTING, false);
    GL_Capability_Switch blnd_on (GL_BLEND,    true);
    GL_Boolean_Holder    dbuf_off(GL_DEPTH_WRITEMASK, false, glDepthMask);
    GL_Enum_Holder       beq_max (GL_BLEND_EQUATION_RGB, GL_MAX, glBlendEquation);
    rd->Render(rd->GetLensRnr(*T.mExplosions));
  }

  // Handle Extendio sound-effects.
  {
    hExt2AlSrc_i i = mExtendioSounds.begin();
    while (i != mExtendioSounds.end())
    {
      if (i->second->IsPlaying())
      {
	i->second->ref_trans().SetFromArray(i->first->RefLastTrans());
	i->second->MarkStampReqTrans();
	rd->Render(rd->GetLensRnr(i->second));
	++i;
      }
      else
      {
	hExt2AlSrc_i j = i; ++i;
	PI.RelinquishAlSource(j->second);
	mExtendioSounds.erase(j);
      }
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

//==============================================================================

void TringuRep_GL_Rnr::ExtendioExploding(Extendio* ext, ExtendioExplosion* exp)
{
  static const Exc_t _eh("TringuRep_GL_Rnr::ExtendioExploding ");

  // printf("%sextendio %s, explosion %s.\n", _eh.Data(), ext->Identify().Data(), exp->Identify().Data());

  // Need to do better:
  AlBuffer *buf = find_sound("BigExplosion");
  if (!buf)
    throw _eh + "explode buffer not found.";

  // Check if pupil is visible / has focus !!!

  TSPupilInfo &PI  = * mTringuRep->GetPupilInfo();
  AlSource    *src = PI.AcquireAlSource();
  if (src)
  {
    src->ref_trans().SetFromArray(ext->RefLastTrans());
    src->QueueBuffer(buf);
    src->SetPitch(buf->GetDuration() / exp->GetExplodeDuration());
    src->Play();
    mExtendioSounds.insert(make_pair(ext, src));
  }

  // This should really go somewhere else -- in TringuRep and in TSPupilInfo.
  // Tringula needs to do direct notification about Extendio destruction
  // to TringuRep -> which can then pass stuf to TSPupilInfo.
  //
  // Normal camera should probably switch to somewhere above the dying extendio.
  {
    GLensWriteHolder lck(&PI);
    ExtendioSpiritio *es = dynamic_cast<ExtendioSpiritio*>(PI.GetCurrentSpiritio());
    if (es && es->GetExtendio() == ext)
      PI.SetCurrentSpiritio(0);

    if (PI.GetSelection()->Has(ext))
    {
      PI.SelectTopMenuByName("MainMenu");
      PI.GetSelection()->Remove(ext);
    }
  }
}

void TringuRep_GL_Rnr::ExtendioDying(Extendio* ext)
{
  // printf("TringuRep_GL_Rnr::ExtendioDying %s.\n", ext->Identify().Data());

  TSPupilInfo  &PI = * mTringuRep->GetPupilInfo();
  hExt2AlSrc_ip ip = mExtendioSounds.equal_range(ext);
  for (hExt2AlSrc_i i = ip.first; i != ip.second; ++i)
  {
    PI.RelinquishAlSource(i->second);
  }
  mExtendioSounds.erase(ip.first, ip.second);
}

void TringuRep_GL_Rnr::ExtendioSound(Extendio* ext, const TString& effect)
{
  static const Exc_t _eh("TringuRep_GL_Rnr::ExtendioSound ");

  // printf("%sNew sound for %s, %s\n", _eh.Data(), ext->Identify().Data(), effect.Data());

  AlBuffer *buf = find_sound(effect);
  if (!buf)
    throw _eh + "explode buffer not found.";

  // Check if pupil is visible / has focus !!!

  TSPupilInfo &PI  = * mTringuRep->GetPupilInfo();
  AlSource    *src = PI.AcquireAlSource();
  if (src)
  {
    src->ref_trans().SetFromArray(ext->RefLastTrans());
    src->QueueBuffer(buf);
    // src->SetPitch(buf->GetDuration() / exp->GetExplodeDuration());
    src->Play();
    mExtendioSounds.insert(make_pair(ext, src));
  }
}


//==============================================================================
// TringulaSpy
//==============================================================================

TringuRep_GL_Rnr::TringulaSpy::TringulaSpy(OptoStructs::ZGlassImg* i, TringuRep_GL_Rnr* m) :
  OptoStructs::A_View(i), mMaster(m)
{}

TringuRep_GL_Rnr::TringulaSpy::~TringulaSpy()
{
  mMaster->mTringulaSpy = 0;
}

void TringuRep_GL_Rnr::TringulaSpy::AbsorbRay(Ray& ray)
{
  if (ray.fRQN < RayNS::RQN_user_0)
    return;

  switch (ray.fRQN)
  {
    case Tringula::PRQN_extendio_exploding:
    {
      ID_t extid = GledNS::ReadLensID(*ray.fCustomBuffer);
      ID_t expid = GledNS::ReadLensID(*ray.fCustomBuffer);
      ray.ResetCustomBuffer();

      Extendio *ext = dynamic_cast<Extendio*>(fImg->fEye->DemangleID2Lens(extid));
      ExtendioExplosion *exp = dynamic_cast<ExtendioExplosion*>(fImg->fEye->DemangleID2Lens(expid));
      mMaster->ExtendioExploding(ext, exp);
      break;
    }
    case Tringula::PRQN_extendio_dying:
    {
      ID_t extid = GledNS::ReadLensID(*ray.fCustomBuffer);
      ray.ResetCustomBuffer();

      Extendio *ext = dynamic_cast<Extendio*>(fImg->fEye->DemangleID2Lens(extid));
      mMaster->ExtendioDying(ext);
      break;
    }
    case Tringula::PRQN_extendio_sound:
    {
      ID_t extid = GledNS::ReadLensID(*ray.fCustomBuffer);
      TString effect;
      *ray.fCustomBuffer >> effect;
      ray.ResetCustomBuffer();

      Extendio *ext = dynamic_cast<Extendio*>(fImg->fEye->DemangleID2Lens(extid));
      mMaster->ExtendioSound(ext, effect);
      break;
    }
    default:
      break;
  }
}
