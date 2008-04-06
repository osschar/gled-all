// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AEVSphereSSpace
//
//

#include "AEVSphereSSpace.h"
#include "AEVSite.h"
#include "AEVSiteViz.h"
#include "AEVSphereSSpace.c7"

#include <Glasses/ZQueen.h>

#include <TMath.h>

ClassImp(AEVSphereSSpace)

/**************************************************************************/

void AEVSphereSSpace::_init()
{
  // ZNode::OM stuffe
  bUseOM = true; mOM = 1;
}

/**************************************************************************/

Bool_t AEVSphereSSpace::ImportSite(AEVSite* site, Bool_t warn)
{
  // Here create site-viz for site if within limits.

  static const Exc_t _eh("AEVSphereSSpace::ImportAEVSite ");

  Float_t t = site->GetLatitude()  /  90 * TMath::PiOver2();
  Float_t p = site->GetLongitude() / 180 * TMath::Pi();

  if(TMath::Abs(t) > TMath::PiOver2() || TMath::Abs(p) > TMath::Pi()) {
    if(warn)
      ISmess(_eh + GForm("site %s (%f,%f) outside limits of %s.",
			 site->GetName(), t, p, GetName()));
    return false;
  }

  AEVSiteViz* sv = new AEVSiteViz(site->GetName(), GForm("SiteViz of %s", site->GetName()));
  sv->RotateLF(3, 1, TMath::PiOver2());
  sv->RotateLF(1, 2, p);
  sv->RotateLF(2, 3, TMath::PiOver2() - t);
  sv->MoveLF(3, 1.001);

  mQueen->CheckIn(sv);
  Add(sv);

  return true;
}

/**************************************************************************/
