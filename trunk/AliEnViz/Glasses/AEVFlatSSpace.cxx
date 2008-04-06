// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AEVFlatSSpace
//
//

#include "AEVFlatSSpace.h"
#include "AEVSite.h"
#include "AEVSiteViz.h"
#include "AEVFlatSSpace.c7"

#include <Glasses/ZQueen.h>

#include <TMath.h>

ClassImp(AEVFlatSSpace)

/**************************************************************************/

void AEVFlatSSpace::_init()
{
  // ZNode::OM stuffe
  bUseOM = true; mOM = 1.5;

  mTheta0 = 0; mDTheta = 180;
  mPhi0   = 0; mDPhi   = 360;
}

/**************************************************************************/

Bool_t AEVFlatSSpace::ImportSite(AEVSite* site, Bool_t warn)
{
  // Here create site-viz for site if within limits.

  static const Exc_t _eh("AEVFlatSSpace::ImportAEVSite ");

  Float_t t = site->GetLatitude(), p = site->GetLongitude();

  Float_t v = (t - mTheta0) / mDTheta;
  Float_t u =  (p - mPhi0)  / mDPhi;

  if(TMath::Abs(u) > 0.5 || TMath::Abs(v) > 0.5) {
    if(warn)
      ISmess(_eh + GForm("site %s (%f,%f) outside limits of %s.",
			 site->GetName(), t, p, GetName()));
    return false;
  }

  AEVSiteViz* sv = new AEVSiteViz(site->GetName(), GForm("SiteViz of %s", site->GetName()));
  sv->SetPos(u*mULen, v*mVLen, 0.001);

  mQueen->CheckIn(sv);
  Add(sv);

  return true;
}

void AEVFlatSSpace::ClearSiteVizes()
{
  list<AEVSiteViz*> vizen;
  CopyListByGlass<AEVSiteViz>(vizen);
  for(list<AEVSiteViz*>::iterator v=vizen.begin(); v!=vizen.end(); ++v) {
    RemoveAll(*v);
  }
}

/**************************************************************************/

void AEVFlatSSpace::ImportAllSites()
{
  static const Exc_t _eh("AEVFlatSSpace::ImportAllSites ");

  ZList* sites = dynamic_cast<ZList*>
    (mQueen->FindLensByPath("AlienUI->Sites"));
  if(sites == 0) {
    throw(_eh + "could not retrive the site list.");
  }

  list<AEVSite*> ss; sites->CopyListByGlass<AEVSite>(ss);
  for(list<AEVSite*>::iterator i = ss.begin(); i!=ss.end(); ++i) {
    ImportSite(*i);
  }
}

/**************************************************************************/
