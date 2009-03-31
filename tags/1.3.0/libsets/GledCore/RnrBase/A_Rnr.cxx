// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "A_Rnr.h"
#include "RnrDriver.h"

#include <Eye/Ray.h>

namespace GNS  = GledNS;
namespace GVNS = GledViewNS;
namespace OS = OptoStructs;

const int A_Rnr::sMaxRnrLevel = 7;

/**************************************************************************/

void A_Rnr::AbsorbRay(Ray& ray)
{
  if(ray.fEyeBits & Ray::EB_StructuralChange)
    DropRnrScheme();
}

/**************************************************************************/

void A_Rnr::crs_links(RnrDriver* rd, RnrScheme* sch)
{
  // Fill links and links as lists.
  OS::lZLinkDatum_i ldatum = fImg->fLinkData.begin();
  OS::lZLinkDatum_i ldend  = fImg->fLinkData.end();
  for(; ldatum != ldend; ++ldatum) {
    if(ldatum->fToGlass == 0) continue;
    OS::ZGlassImg* limg = ldatum->GetToImg();
    const GNS::RnrBits& rb = ldatum->GetLinkInfo()->fDefRnrBits;

    if(rb.SelfOn()) {
      rd->FillRnrScheme(sch, rd->GetRnr(limg), rb);
    }

    if(limg->fIsList && rb.ListOn()) {
      mRnrScheme->AddListChangeObserver(limg, this);
      rd->FillRnrScheme(sch, limg->GetElementImgs(), rb);
    }
  }
}

void A_Rnr::crs_self(RnrDriver* rd, RnrScheme* sch)
{
  // Fill self.
  GNS::RnrBits& rb(fImg->GetCI()->fDefRnrCtrl.fRnrBits);
  if(rb.SelfOn()) {
    rd->FillRnrScheme(sch, this, rb);
  }
}

void A_Rnr::crs_elements(RnrDriver* rd, RnrScheme* sch)
{
  // Fill self as list.
  GNS::RnrBits& rb(fImg->GetCI()->fDefRnrCtrl.fRnrBits);
  if(fImg->fIsList && rb.ListOn()) {
    rd->FillRnrScheme(sch, fImg->GetElementImgs(), rb);
  }
}

void A_Rnr::CreateRnrScheme(RnrDriver* rd)
{
  crs_links(rd, mRnrScheme);
  crs_self(rd, mRnrScheme);
  crs_elements(rd, mRnrScheme);
}



void A_Rnr::DropRnrScheme()
{
  if(mRnrScheme) {
    delete mRnrScheme;
    mRnrScheme = 0;
  }
}

/**************************************************************************/
// A_Rnr::Fl_Event

namespace {
  const char* fl_event_names[] = {
    "NO_EVENT",
    "PUSH",
    "RELEASE",
    "ENTER",
    "LEAVE",
    "DRAG",
    "FOCUS",
    "UNFOCUS",
    "KEYDOWN",
    "KEYUP",
    "CLOSE",
    "MOVE",
    "SHORTCUT",
    "DEACTIVATE",
    "ACTIVATE",
    "HIDE",
    "SHOW",
    "PASTE",
    "SELECTIONCLEAR",
    "MOUSEWHEEL",
    "DND_ENTER",
    "DND_DRAG",
    "DND_LEAVE",
    "DND_RELEASE"
  };
}

void A_Rnr::Fl_Event::dump()
{
  printf("%s: key=%d but=%d state=0x%x text='%s' len=%d\n",
	 fl_event_names[fEvent], fKey, fButton, fState,
	 fText.Data(), fText.Length());
}

/**************************************************************************/
// ListChangeObserver
/**************************************************************************/

void ListChangeObserver::AbsorbRay(Ray& ray)
{
  using namespace RayNS;
  if(ray.fRQN > RQN_list_begin && ray.fRQN < RQN_list_end )
    mRnr->DropRnrScheme();
}

/**************************************************************************/
// RnrScheme
/**************************************************************************/

RnrScheme::~RnrScheme()
{
  for(OS::lpA_View_i i=fListObservers.begin(); i!=fListObservers.end(); ++i)
    delete *i;
}

void RnrScheme::AddListChangeObserver(OS::ZGlassImg* list, A_Rnr* client)
{
  fListObservers.push_back(new ListChangeObserver(list, client));
}
