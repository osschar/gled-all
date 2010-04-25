// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "OptoStructs.h"
#include "Eye.h"

#include <Glasses/AList.h>

using namespace OptoStructs;
namespace GNS  = GledNS;
namespace GVNS = GledViewNS;

/**************************************************************************/
// ZGlassImg
/**************************************************************************/

ZGlassImg::ZGlassImg(Eye* e) : fEye(e), fLens(0)
{
  // Constructor for null-lens-image.
  // One exists in every eye to denote empty list slots when needed.

  fIsList       = false;
  fElementImgs  = 0;
  fDefRnr       = 0;
  fFullMTW_View = 0;
}

ZGlassImg::ZGlassImg(Eye* e, ZGlass* lens) : fEye(e), fLens(lens)
{
  fIsList = dynamic_cast<AList*>(lens) ? true : false;
  fElementImgs  = 0;
  fDefRnr       = 0;
  fFullMTW_View = 0;

  ZGlass::lLinkRep_t lreps; lens->CopyLinkReps(lreps);
  for(ZGlass::lLinkRep_i i = lreps.begin(); i != lreps.end(); ++i) {
    fLinkData.push_back(ZLinkDatum(this, *i));
  }
}

ZGlassImg::~ZGlassImg()
{
  delete fElementImgs;

  // The destructor of A_View removes itself from the fViews list.
  // To by-pass this (and image auto-destruction) fViews is
  // wiped before destruction.
  // This condition is checked in CheckOutView();
  lpA_View_t views;
  views.swap(fViews);
  for (lpA_View_i i = views.begin(); i != views.end(); ++i)
    delete *i;
}

/**************************************************************************/

void ZGlassImg::PreAbsorption(Ray& ray)
{
  using namespace RayNS;

  if (ray.fRQN == RQN_link_change)
  {
    for (lZLinkDatum_i i=fLinkData.begin(); i!=fLinkData.end(); ++i)
    {
      if (i->fToGlass != i->fLinkRep.fLinkRef)
      {
	i->fToGlass = i->fLinkRep.fLinkRef;
	i->fToImg   = 0;
      }
    }
  }
  else if (ray.fRQN > RQN_list_begin && ray.fRQN < RQN_list_end && fElementImgs)
  {
    // XXX April 2010. Is this always necessary? Maybe could optimize for
    // certain list operations. Rebuild can get expensive for LARGE lists.
    delete fElementImgs;
    fElementImgs = 0;
  }
}

void ZGlassImg::PostAbsorption(Ray& ray)
{}

void ZGlassImg::CheckInView(A_View* v)
{
  fViews.push_back(v);
}

void ZGlassImg::CheckOutView(A_View* v)
{
  // This happens during direct destruction of ZGlassImg.
  if (fViews.empty())
    return;

  fViews.remove(v);
  if (fViews.empty())
    fEye->RemoveImage(this);
}

/**************************************************************************/

ZLinkDatum* ZGlassImg::GetLinkDatum(const TString& lnk)
{
  for(lZLinkDatum_ri i=fLinkData.rbegin(); i!=fLinkData.rend(); ++i) {
    if(i->fLinkRep.fLinkInfo->fName == lnk) return &(*i);
  }
  return 0;
}

lpZGlassImg_t* ZGlassImg::GetElementImgs()
{
  if(!fIsList) return 0;
  if(fElementImgs) return fElementImgs;
  fElementImgs = new lpZGlassImg_t;
  AList* l = (AList*)fLens;
  lpZGlass_t d; l->CopyList(d);
  for(lpZGlass_i i=d.begin(); i!=d.end(); ++i) {
    fElementImgs->push_back(fEye->DemanglePtr(*i));
  }
  return fElementImgs;
}

void ZGlassImg::DumpLinkData()
{
  printf("Dumping list of ZLinkDatum's -- size = %lu\n", fLinkData.size());
  for(lZLinkDatum_i i=fLinkData.begin(); i!=fLinkData.end(); ++i) {
    printf("  img=%p, linkref=%p, linkinfo=%p\n",
	   i->fImg, i->fLinkRep.fLinkRef, i->fLinkRep.fLinkInfo);
  }
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
