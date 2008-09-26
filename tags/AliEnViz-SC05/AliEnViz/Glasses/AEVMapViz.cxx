// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AEVMapViz
//
//

#include "AEVMapViz.h"
#include <Glasses/ZImage.h>
#include "AEVSite.h"
#include "AEVMapViz.c7"
#include "AEVSiteViz.h"

#include <Glasses/AEVFlatSSpace.h>
#include <Glasses/AEVSite.h>
#include <Glasses/ZQueen.h>

ClassImp(AEVMapViz)

/**************************************************************************/

void AEVMapViz::_init()
{}

/**************************************************************************/

namespace {

  struct sub_map {
    const char* n;
    float x0, y0, dx, dy;
    float sx;
    float ox, oy;
  };

  // Due to cosmic interferences this is scaled as (xs,ys) = (7,3.5).
  sub_map sub_map_list[] = {
    { "Europe",	 0.14,  0.95, 0.55, 0.55, 0,    0,    0 },
    { "Russia",	 0.66,  1.12, 0.25, 0.20, 1,    0,    0 },
    { "NE-USA",	-1.62,  0.82, 0.20, 0.20, 0.5, -0.8, -0.2 },
    { "SE-USA",	-1.83,  0.60, 0.20, 0.20, 0.5, -0.8, -0.2 },
    { "NW-USA",	-2.39,  0.73, 0.20, 0.20, 0.5, -0.8, -0.2 },
    { 0 }
  };

}

void AEVMapViz::CutEarth_PDC04(ZImage* tex)
{
  // Creates chunks of Earth-map relevant for Alice PDC04.
  // Uses static (hardcoded) data to produce visually pleasing result.
  // Should be called at initialization time ... not MIR-safe, but
  // could easily be rewritten in this manner.

  const Float_t Sx = 7, Sy = 3.5;

  if(mNameSwitch == 0) {
    ZRlNodeMarkup* znm = new ZRlNodeMarkup;
    znm->SetTileCol(0, 0, 0.4, 0.4);
    mQueen->CheckIn(znm);
    SetNameSwitch(znm);
  }

  sub_map& o = sub_map_list[0];
  sub_map* smp = sub_map_list;
  while(smp->n) {
    sub_map &s = *smp;

    Float_t llx = s.x0 - s.dx/2, urx = s.x0 + s.dx/2;
    Float_t lly = s.y0 - s.dy/2, ury = s.y0 + s.dy/2;

    AEVFlatSSpace* fss = new
      AEVFlatSSpace(s.n, GForm("SSpace chunk representing %s", s.n));

    fss->SetRnrMod(*mNameSwitch);
    fss->SetModSelf(false);

    fss->SetTexture(tex);
    //fss->SetTexX0((llx+Sx/2)/Sx);  fss->SetTexY0(1 - (ury+Sy/2)/Sy);
    //fss->SetTexX1((urx+Sx/2)/Sx);  fss->SetTexY1(1 - (lly+Sy/2)/Sy);
    fss->SetTexX0((llx+Sx/2)/Sx);  fss->SetTexY0((lly+Sy/2)/Sy);
    fss->SetTexX1((urx+Sx/2)/Sx);  fss->SetTexY1((ury+Sy/2)/Sy);
    fss->SetUV(s.dx, s.dy);
    fss->SetPhi0(360*s.x0/Sx);   fss->SetDPhi(360*s.dx/Sx);
    fss->SetTheta0(180*s.y0/Sy); fss->SetDTheta(180*s.dy/Sy);

    if(smp != sub_map_list) {
      fss->SetPos((s.x0 - o.x0 - s.ox)*s.sx, (s.y0 - o.y0 - s.oy), 0);
    }

    // Here use ZQueen::IncarnateWAttach & suppress flare broadcast.
    mQueen->CheckIn(fss);
    Add(fss);

    ++smp;
  }
}

/**************************************************************************/

Bool_t AEVMapViz::ImportSite(AEVSite* site, Bool_t warn)
{
  static const Exc_t _eh("AEVMapViz::ImportSite ");

  GMutexHolder lstlck(mListMutex);
  Stepper<AEVSSpace_ABase> s(this);
  while(s.step()) {
    if(s->ImportSite(site, warn))
      return true;
  }
  return false;
}

void AEVMapViz::PopulateWSites(Bool_t warn)
{
  static const Exc_t _eh("AEVMapViz::PopulateWSites ");

  if(mSites == 0) {
    throw(_eh + "link Sites must be set.");
  }

  list<AEVSite*> sites;
  mSites->CopyListByGlass<AEVSite>(sites);

  GMutexHolder lstlck(mListMutex);
  Stepper<AEVSSpace_ABase> s(this);
  while(s.step()) {
    for(list<AEVSite*>::iterator i=sites.begin(); i!=sites.end(); ++i) {
      if(s->ImportSite(*i, warn)) {
	list<AEVSite*>::iterator j = i--;
	sites.erase(j);
      }
    }
  }
}

void AEVMapViz::ClearSiteVizes()
{
  list<AEVSSpace_ABase*> space_chunks;
  CopyListByGlass<AEVSSpace_ABase>(space_chunks);
  for(list<AEVSSpace_ABase*>::iterator c=space_chunks.begin(); c!=space_chunks.end(); ++c) {
    (*c)->ClearSiteVizes();
  }
}

/**************************************************************************/

void AEVMapViz::RnrSiteNamesOn()
{
  if(mNameSwitch != 0)
    mNameSwitch->SetRnrNames(true);
}

void AEVMapViz::RnrSiteNamesOff()
{
  if(mNameSwitch != 0)
    mNameSwitch->SetRnrNames(false);
}

/**************************************************************************/

AEVSiteViz* AEVMapViz::FindSiteViz(const Text_t* name)
{
  list<ZList*> space_chunks;
  CopyListByGlass<ZList>(space_chunks);
  for(list<ZList*>::iterator c=space_chunks.begin(); c!=space_chunks.end(); ++c) {
    AEVSiteViz* sv = dynamic_cast<AEVSiteViz*>((*c)->GetElementByName(name));
    if(sv != 0) return sv;
  }
  return 0;
}
