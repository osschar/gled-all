// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "A_Rnr.h"
#include "RnrDriver.h"

#include <Net/Ray.h>

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
    const GVNS::RnrBits& rb = ldatum->GetLinkInfo()->fViewPart->fDefRnrBits;

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
  GVNS::RnrBits& rb(fImg->fClassInfo->fViewPart->fDefRnrCtrl.fRnrBits);
  if(rb.SelfOn()) {
    rd->FillRnrScheme(sch, this, rb);
  }
}

void A_Rnr::crs_elements(RnrDriver* rd, RnrScheme* sch)
{
  // Fill self as list.
  GVNS::RnrBits& rb(fImg->fClassInfo->fViewPart->fDefRnrCtrl.fRnrBits);
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
