// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TSPupilInfo.h"
#include <Glasses/ZDeque.h>
#include <Glasses/ZVector.h>
#include <Glasses/Scene.h>
#include <Glasses/WGlWidget.h>
#include <Glasses/TimeMaker.h>
#include "Extendio.h"
#include "Spiritio.h"
#include "TringuRep.h"
#include "TSPupilInfo.c7"

#include "ExtendioSpiritio.h"

#include <Glasses/Camera.h>
#include <Glasses/AlSource.h>

// TSPupilInfo

//______________________________________________________________________________
//
// Sub-class of PupilInfo adding functionality needed for operation of
// camera, overlay and event-handlers in greed demo.
//
// TS ~ TringuSol
//
// The role of TringuRep link is half cooked. It should in principle point to
// the primary Tringula.

ClassImp(TSPupilInfo);

Int_t TSPupilInfo::sNMaxAlSources = 32;

//==============================================================================

void TSPupilInfo::_init()
{
  // From PupilInfo -- restrict Overlay and EventHandler fids.
  mOverlay_fid      = ZHashList::FID();
  mEventHandler_fid = ZHashList::FID();

  mSelectionColor.rgba(1, 0.125, 0.25);

  mZFovPreSpiritio = 60;

  // AlSource-pool vars initialised in AdEnlightenment().
}

TSPupilInfo::TSPupilInfo(const Text_t* n, const Text_t* t) :
  PupilInfo(n, t)
{
  _init();
}

TSPupilInfo::~TSPupilInfo()
{}

//==============================================================================

void TSPupilInfo::AdEnlightenment()
{
  // Create overlay and event-handler lists.

  PARENT_GLASS::AdEnlightenment();

  if (mSelection == 0) {
    assign_link<ZHashList>(mSelection, FID(), "Selection", "Selection of TSPupilInfo");
    mSelection->SetElementFID(Extendio::FID());
  }
  if (mOverlay == 0)
  {
    assign_link<ZHashList>(mOverlay, FID(), "Overlay", "Overlay list of TSPupilInfo");
  }
  if (mMenuScene == 0)
  {
    assign_link<Scene>(mMenuScene, FID(), "MenuScene", "Menu overlay of TSPupilInfo");
    mMenuScene->SetMIRActive(false);
    AddOverlayElement(*mMenuScene);
    mMenuScene->MakeRnrModList();
  }
  if (mSpiritioScene == 0)
  {
    assign_link<Scene>(mSpiritioScene, FID(), "SpiritioScene", "Spiritio overlay of TSPupilInfo");
    mSpiritioScene->SetMIRActive(false);
    AddOverlayElement(*mSpiritioScene);
    mSpiritioScene->MakeRnrModList();
  }

  // Local AlSources.
  mNAlSources = 0;
  if (mAlSources == 0)
  {
    assign_link<ZVector>(mAlSources, FID(), "AlSources");
    mAlSources->SetMIRActive(false);
    mAlSources->SetElementFID(AlSource::FID());
  }
  if (mAlSourcesFree == 0)
  {
    assign_link<ZDeque>(mAlSourcesFree, FID(), "FreeAlSources");
    mAlSourcesFree->SetMIRActive(false);
    mAlSourcesFree->SetElementFID(AlSource::FID());
  }
  if (mAlSourcesUsed == 0)
  {
    assign_link<ZHashList>(mAlSourcesUsed, FID(), "UsedAlSources");
    mAlSourcesUsed->SetMIRActive(false);
    mAlSourcesUsed->SetElementFID(AlSource::FID());
  }

  // From PupilInfo
  if (mEventHandler == 0)
  {
    assign_link<ZHashList>(mEventHandler, FID(), "Event handlers", "Event handlers of TSPupilInfo");
  }
}

//==============================================================================

void TSPupilInfo::TimeTick(Double_t t, Double_t dt)
{
  if (*mCurrentSpiritio)
    mCurrentSpiritio->TimeTick(t, dt);
}

//==============================================================================

void TSPupilInfo::SelectExtendio(Extendio* ext, Bool_t multiple)
{
  if (multiple)
  {
    if (ext)
    {
      if (mSelection->Has(ext))
	mSelection->Remove(ext);
      else
	mSelection->PushBack(ext);
      //ext->SetSelected(!ext->GetSelected());
    }
  }
  else
  {
    mSelection->ClearList();
    if (ext)
    {
      mSelection->PushBack(ext);
    }
  }

  SelectTopMenuForLens(ext);
}

//==============================================================================

void TSPupilInfo::AddOverlayElement(ZGlass* l)
{
  ZHashList* list = (ZHashList*) mOverlay.get();
  list->PushFront(l);
}

void TSPupilInfo::RemoveOverlayElement(ZGlass* l)
{
  ZHashList* list = (ZHashList*) mOverlay.get();
  list->Remove(l);
}

//==============================================================================

void TSPupilInfo::AddEventHandler(ZGlass* l)
{
  ZHashList* list = (ZHashList*) mEventHandler.get();
  list->PushFront(l);
}

void TSPupilInfo::RemoveEventHandler(ZGlass* l)
{
  ZHashList* list = (ZHashList*) mEventHandler.get();
  list->Remove(l);
}

//==============================================================================

// Maybe we need a list of spiritios?
// Who knows ... only one can have camera though.
// Then it can be called SpiritioVisionario, or sth.

void TSPupilInfo::SetCurrentSpiritio(Spiritio* s)
{
  // Expected to be called under write-lock.

  if (s == 0)
    s = *mDefaultSpiritio;

  Spiritio *exs = *mCurrentSpiritio;

  if (exs)
  {
    {
      GLensReadHolder slck(exs);
      exs->Deactivate();
      exs->SetPupilInfo(0);
    }

    mSpiritioScene->RemoveAll(exs);
    RemoveEventHandler(exs);
  }
  else
  {
    mZFovPreSpiritio = mZFov;
  }

  set_link_or_die(mCurrentSpiritio.ref_link(), s, FID());

  if (s)
  {
    mSpiritioScene->PushFront(s);
    AddEventHandler(s);

    {
      GLensReadHolder slck(s);
      s->SetPupilInfo(this);
      s->Activate();
    }

    SetCameraBase(s->GetCamera());
    SetUpReference(0);
    EmitCameraHomeRay();
  }
  else
  {
    SetZFov(mZFovPreSpiritio);
    SetCameraBase(0);
  }
}

void TSPupilInfo::EnactExtendioSpiritio(Extendio* ext)
{
  static const Exc_t _eh("TSPupilInfo::EnactExtendioSpiritio ");

  assert_MIR_presence(_eh, MC_IsDetached);

  AList *uidir = dynamic_cast<AList*>(mQueen->FindLensByPath("var/glassui"));
  if (!uidir)
    throw _eh + "uidir not found.";

  GledNS::ClassInfo *ci = ext->VGlassInfo();
  while (ci)
  {
    // printf("Trying to get spiritio for class '%s'\n", ci->fName.Data());
    AList *gdir = dynamic_cast<AList*>(uidir->GetElementByName(ci->fName));
    if (gdir)
    {
      ExtendioSpiritio *s = dynamic_cast<ExtendioSpiritio*>(gdir->GetElementByName("spiritio"));
      if (s)
      {
	// printf("Found spiritio: %p, class='%s'.\n", s, s->ClassName());
	{
	  GLensReadHolder slck(s);
	  s->SetExtendio(ext);
	}
	{
	  GLensReadHolder slck(this);
	  SetCurrentSpiritio(s);
	}

	return;
      }
    }
    ci = ci->GetParentCI();
  }

  throw _eh + "No suitable spiritio found.";
}


//==============================================================================
// Menu handling
//==============================================================================

WGlWidget* TSPupilInfo::FindMenuEntry(const TString& name)
{
  return dynamic_cast<WGlWidget*>(mMenuScene->GetElementByName(name));
}

void TSPupilInfo::SelectTopMenu(WGlWidget* weed)
{
  if (mLastMenu.is_set())
    mLastMenu->SetRnrElements(false);
  if (weed)
    weed->SetRnrElements(true);
  SetLastMenu(weed);
}

void TSPupilInfo::SelectTopMenuByName(const TString& name)
{
  SelectTopMenu(FindMenuEntry(name));
}

void TSPupilInfo::SelectTopMenuForLens(ZGlass* lens)
{
  static const Exc_t _eh("TSPupilInfo::SelectTopMenuForLens ");

  if (lens == 0)
  {
    SelectTopMenuByName("MainMenu");
    return;
  }

  WGlWidget *lm = FindMenuEntry("LensMenu");
  if (lm == 0)
    throw _eh + "can not get lens menu.";

  AList *uidir = dynamic_cast<AList*>(mQueen->FindLensByPath("var/glassui"));
  if (!uidir)
    throw _eh + "uidir not found.";

  lm->ClearList();

  // Descend towards parent classes, merge elements.
  GledNS::ClassInfo *ci = lens->VGlassInfo();
  while (ci)
  {
    AList *gdir = dynamic_cast<AList*>(uidir->GetElementByName(ci->fName));
    if (gdir)
    {
      AList *l = dynamic_cast<AList*>(gdir->GetElementByName("menu"));
      if (l)
      {
	lpZGlass_t mes;
	l->CopyList(mes);
	for (lpZGlass_ri i = mes.rbegin(); i != mes.rend(); ++i)
	{
	  lm->PushFront(*i);
	}
      }
    }
    ci = ci->GetParentCI();
  }
  lm->SetDaughterCbackStuff(lens);

  // Organize them on the grid.
  mGridStepper.Reset();
  Stepper<ZNode> s(lm);
  while (s.step())
  {
    mGridStepper.SetNodeAdvance(*s);
  }

  SelectTopMenu(lm);
}


//==============================================================================
// Local AlSource management
//==============================================================================

// Locking is not strictly necessary as access is only expected from the
// fltk thread.

AlSource* TSPupilInfo::AcquireAlSource()
{
  static const Exc_t _eh("TSPupilInfo::AcquireAlSource ");

  GMutexHolder _lck(mAlSourceMutex);

  AlSource *src = 0;
  if (mAlSourcesFree->IsEmpty())
  {
    if (mNAlSources < sNMaxAlSources)
    {
      GLensWriteHolder _wlck(this);
      src = new AlSource(GForm("AlSource %d", mNAlSources));
      mQueen->CheckIn(src);
      mAlSources->Add(src);
      ++mNAlSources;
    }
    else
    {
      // Could take a used one and recylcle it:
      //   src->Stop();
      //   src->UnqueueAllBuffers();
      // Need transient source category ... and a way to steal it
      // back from whoever has it.
      // Still, returning 0 is some cases seems unavoidable.
      ISwarn(_eh + "No free AlSources, maximum number reached.");
      return 0;
    }
  }
  else
  {
    src = (AlSource*) mAlSourcesFree->PopFront();
  }
  mAlSourcesUsed->PushBack(src);
  return src;
}

void TSPupilInfo::RelinquishAlSource(AlSource* src)
{
  GMutexHolder _lck(mAlSourceMutex);

  if (src->IsPlaying())
    src->Stop();
  src->UnqueueAllBuffers();
  src->SetPitch(1);
  
  mAlSourcesUsed->Remove(src);
  mAlSourcesFree->PushBack(src);
}
