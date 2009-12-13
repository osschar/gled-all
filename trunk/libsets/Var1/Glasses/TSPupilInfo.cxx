// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TSPupilInfo.h"
#include <Glasses/Scene.h>
#include <Glasses/WGlWidget.h>
#include <Glasses/TimeMaker.h>
#include "Spiritio.h"
#include "TSPupilInfo.c7"

#include <Glasses/Camera.h>

// TSPupilInfo

//______________________________________________________________________________
//
// Sub-class of PupilInfo adding functionality needed for operation of
// camera, overlay and event-handlers in greed demo.
//
// TS ~ TringuSol

ClassImp(TSPupilInfo);

//==============================================================================

void TSPupilInfo::_init()
{
  // From PupilInfo -- restrict Overlay and EventHandler fids.
  mOverlay_fid      = ZHashList::FID();
  mEventHandler_fid = ZHashList::FID();
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
