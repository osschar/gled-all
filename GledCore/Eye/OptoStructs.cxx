// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include <Glasses/ZList.h>
#include <Net/Ray.h>

#include "OptoStructs.h"
#include "Eye.h"

using namespace OptoStructs;
namespace GNS  = GledNS;
namespace GVNS = GledViewNS;

/**************************************************************************/
// ZGlassImg
/**************************************************************************/

ZGlassImg::ZGlassImg(Eye* e, ZGlass* g) : fEye(e), fGlass(g) {
  fClassInfo = g->VGlassInfo();
  fIsList = dynamic_cast<ZList*>(g) ? true : false;
  fElementImgs   = 0;
  fDefRnr       = 0;
  fFullMTW_View = 0;
 
  ZGlass::lLinkRep_t lreps; g->CopyLinkReps(lreps);
  for(ZGlass::lLinkRep_i i = lreps.begin(); i != lreps.end(); ++i) {
    fLinkData.push_back(ZLinkDatum(this, *i));
  }
}

ZGlassImg::~ZGlassImg() {
  while(!fViews.empty()) {
    // The destructor of A_View removes itself from the fViews list.
    delete fViews.back();
  }
}

/**************************************************************************/

void ZGlassImg::PreAbsorption(Ray& ray)
{
  using namespace RayNS;

  if(ray.fRQN == RQN_link_change) {
    for(lZLinkDatum_i i=fLinkData.begin(); i!=fLinkData.end(); ++i) {
      if(i->fToGlass != i->fLinkRep.fLinkRef) {
	i->fToGlass = i->fLinkRep.fLinkRef;
	i->fToImg   = 0;
      }
    }
  }

  if(ray.fRQN > RQN_list_begin && ray.fRQN < RQN_list_end && fElementImgs) {
    delete fElementImgs;
    fElementImgs = 0;
  }
}

void ZGlassImg::PostAbsorption(Ray& ray)
{}

/**************************************************************************/

lpZGlassImg_t* ZGlassImg::GetElementImgs()
{
  if(!fIsList) return 0;
  if(fElementImgs) return fElementImgs;
  fElementImgs = new lpZGlassImg_t;
  ZList* l = dynamic_cast<ZList*>(fGlass);
  lpZGlass_t d; l->Copy(d);
  for(lpZGlass_i i=d.begin(); i!=d.end(); ++i) {
    fElementImgs->push_back(fEye->DemanglePtr(*i));
  }  
  return fElementImgs;
}

/**************************************************************************/
// ZLinkDatum
/**************************************************************************/

ZGlassImg* ZLinkDatum::GetToImg()
{
  if(fToImg) return fToImg;
  if(fToGlass) {
    fToImg = fImg->fEye->DemanglePtr(fToGlass);
    return fToImg;
  }
  return 0;
}

/**************************************************************************/
// A_View
/**************************************************************************/

void A_View::SetImg(ZGlassImg* newimg)
{
  if(fImg) fImg->CheckOutView(this);
  fImg = newimg;
  if(fImg) fImg->CheckInView(this);
}
