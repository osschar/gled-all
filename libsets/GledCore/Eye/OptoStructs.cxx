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


//==============================================================================
// ZGlassImg
//==============================================================================

ZGlassImg::ZGlassImg(Eye* e) : fElementImgs(0), fEye(e), fLens(0)
{
  // Constructor for null-lens-image.
  // One exists in every eye to denote empty list slots when needed.

  fRefCount     = 0;
  fDefRnr       = 0;
  fFullMTW_View = 0;
}

ZGlassImg::ZGlassImg(Eye* e, ZGlass* lens) : fElementImgs(0), fEye(e), fLens(lens)
{
  fRefCount     = 0;
  fDefRnr       = 0;
  fFullMTW_View = 0;

  ZGlass::lLinkRep_t lreps; lens->CopyLinkReps(lreps);
  for (ZGlass::lLinkRep_i i = lreps.begin(); i != lreps.end(); ++i)
  {
    fLinkData.push_back(ZLinkDatum(this, *i));
  }
}

ZGlassImg::~ZGlassImg()
{
  static const Exc_t _eh("ZGlassImg::ZGlassImg ");

  if (fElementImgs)
    ClearElementImgs();

  // The destructor of A_View removes itself from the fViews list. To bypass
  // this (and image auto-destruction) fViews is wiped before destruction.
  // This condition is checked in CheckOutView();
  lpA_View_t views;
  views.swap(fViews);

  for (lpA_View_i i = views.begin(); i != views.end(); ++i)
  {
    delete *i;
  }

  // Eye, when destructing an image, increases reference count by one to block
  // auto-destruction. So, at the end, reference count should be one.
  if (fRefCount != 1)
    ISerr(_eh + "ref-count not zero on destruction, this will not end well.");
}

//------------------------------------------------------------------------------

void ZGlassImg::PreAbsorption(Ray& ray)
{
  using namespace RayNS;

  if (ray.fRQN == RQN_link_change)
  {
    for (lZLinkDatum_i i=fLinkData.begin(); i!=fLinkData.end(); ++i)
    {
      if (i->fToGlass != i->fLinkRep.fLinkRef)
      {
	i->ResetToGlass();
      }
    }
  }
  else if (ray.fRQN > RQN_list_begin && ray.fRQN < RQN_list_end && fElementImgs)
  {
    // XXX April 2010. Is this always necessary? Maybe could optimize for
    // certain list operations. Rebuild can get expensive for LARGE lists.
    ClearElementImgs();
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
  if (HasZeroRefCount())
    fEye->ZeroRefCountImage(this);
}

void ZGlassImg::IncRefCount()
{
  ++fRefCount;
}

void ZGlassImg::DecRefCount()
{
  --fRefCount;
  if (HasZeroRefCount())
    fEye->ZeroRefCountImage(this);
}

//------------------------------------------------------------------------------

ZLinkDatum* ZGlassImg::GetLinkDatum(const TString& lnk)
{
  for (lZLinkDatum_ri i = fLinkData.rbegin(); i != fLinkData.rend(); ++i)
  {
    if (i->fLinkRep.fLinkInfo->fName == lnk)
      return &(*i);
  }
  return 0;
}

lpZGlassImg_t* ZGlassImg::GetElementImgs()
{
  if (fElementImgs) return fElementImgs;

  AList* l = GetList();
  if (!l) return 0;

  fElementImgs = new lpZGlassImg_t;
  lpZGlass_t d; l->CopyList(d);
  for (lpZGlass_i i = d.begin(); i != d.end(); ++i)
  {
    ZGlassImg *to_img = fEye->DemanglePtr(*i);
    to_img->IncRefCount();
    fElementImgs->push_back(to_img);
  }
  return fElementImgs;
}

void ZGlassImg::ClearElementImgs()
{
  for (lpZGlassImg_i i = fElementImgs->begin(); i != fElementImgs->end(); ++i)
  {
    (*i)->DecRefCount();
  }
  delete fElementImgs;
  fElementImgs = 0;
}

void ZGlassImg::DumpLinkData()
{
  printf("Dumping list of ZLinkDatum's -- size = %lu\n", fLinkData.size());
  for (lZLinkDatum_i i = fLinkData.begin(); i != fLinkData.end(); ++i)
  {
    printf("  img=%p, linkref=%p, linkinfo=%p\n",
	   i->fImg, i->fLinkRep.fLinkRef, i->fLinkRep.fLinkInfo);
  }
}


//==============================================================================
// ZLinkDatum
//==============================================================================

ZLinkDatum::~ZLinkDatum()
{}

void ZLinkDatum::ResetToGlass()
{
  fToGlass = fLinkRep.fLinkRef;
}

ZGlassImg* ZLinkDatum::GetToImg()
{
  return fToGlass ? fImg->fEye->DemanglePtr(fToGlass) : 0;
}


//==============================================================================
// A_View
//==============================================================================

void A_View::SetImg(ZGlassImg* newimg)
{
  if(fImg) fImg->CheckOutView(this);
  fImg = newimg;
  if(fImg) fImg->CheckInView(this);
}


//==============================================================================
// ZLinkView
//==============================================================================

ZLinkView::ZLinkView(ZLinkDatum* ld) :
  fLinkDatum(ld),
  fToGlass(0)
{
  LinkViewUpdate();
}
