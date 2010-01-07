// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TSPupilInfo.h"
#include <Glasses/Scene.h>
#include <Glasses/WGlWidget.h>
#include <Glasses/TimeMaker.h>
#include "Extendio.h"
#include "Spiritio.h"
#include "TringuRep.h"
#include "TSPupilInfo.c7"

#include <Glasses/Camera.h>

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

//==============================================================================

void TSPupilInfo::_init()
{
  // From PupilInfo -- restrict Overlay and EventHandler fids.
  mOverlay_fid      = ZHashList::FID();
  mEventHandler_fid = ZHashList::FID();

  mSelectionColor.rgba(1, 0.125, 0.25);
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
    ZHashList* l = new ZHashList("Selection", "Selection of TSPupilInfo");
    l->SetElementFID(Extendio::FID());
    mQueen->CheckIn(l);
    SetSelection(l);
  }
  if (mOverlay == 0)
  {
    ZHashList* l = new ZHashList("Overlay", "Overlay list of TSPupilInfo");
    mQueen->CheckIn(l);
    SetOverlay(l);
  }
  if (mMenuScene == 0)
  {
    Scene* s = new Scene("MenuScene", "Menu overlay of TSPupilInfo");
    s->SetMIRActive(false);
    mQueen->CheckIn(s);
    SetMenuScene(s);
    AddOverlayElement(s);
    s->MakeRnrModList();
  }
  if (mSpiritioScene == 0)
  {
    Scene* s = new Scene("SpiritioScene", "Spiritio overlay of TSPupilInfo");
    s->SetMIRActive(false);
    mQueen->CheckIn(s);
    SetSpiritioScene(s);
    AddOverlayElement(s);
    s->MakeRnrModList();
  }

  if (mEventHandler == 0)
  {
    ZHashList* l = new ZHashList("Event handlers", "Event handlers of TSPupilInfo");
    mQueen->CheckIn(l);
    SetEventHandler(l);
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
    SetCameraBase(0);
  }
}

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

  ZGlass *uidir = mQueen->FindLensByPath("var/glassui");
  if (!uidir)
    throw _eh + "uidir not found.";

  lm->ClearList();

  // Descend towards parent classes, merge elements.
  GledNS::ClassInfo *ci = lens->VGlassInfo();
  while (ci)
  {
    ZGlass *gdir = uidir->FindLensByPath(ci->fName);
    if (gdir)
    {
      ZList *l = dynamic_cast<ZList*>(gdir->FindLensByPath("menu"));
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
