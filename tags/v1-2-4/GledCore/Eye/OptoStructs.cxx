// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include <Glasses/ZGlass.h>
#include <Glasses/ZList.h>
#include <Net/Ray.h>

#include "OptoStructs.h"
#include "Eye.h"
#include <RnrBase/A_Rnr.h>
#include <RnrBase/RnrScheme.h>

#include <FL/Fl_Window.H>

#include <algorithm>
#include <iterator>


using namespace OptoStructs;
namespace GNS  = GledNS;
namespace GVNS = GledViewNS;

/**************************************************************************/
// ZGlassImg
/**************************************************************************/

ZGlassImg::ZGlassImg(Eye* e, ZGlass* g) : fEye(e), fGlass(g) {
  fClassInfo = g->VGlassInfo();
  fIsList = dynamic_cast<ZList*>(g) ? true : false;
  fDefView = 0;
  fFullMTW_View = 0;

  /*
  lppZGlass_t links; g->CopyLinkRefs(links);
  auto_ptr<GNS::lpLinkMemberInfo_t> membs( fClassInfo->ProduceFullLinkMemberInfoList() );
  assert(links.size() == membs->size());
  GNS::lpLinkMemberInfo_i li = membs->begin();
  for(lppZGlass_i l=links.begin(); l!=links.end(); ++l, ++li) {
    fLinkData.push_back(ZLinkDatum(*li, this, **l));
  }
  */
  
  ZGlass::lLinkRep_t lreps; g->CopyLinkReps(lreps);
  for(ZGlass::lLinkRep_i i = lreps.begin(); i != lreps.end(); ++i) {
    fLinkData.push_back(ZLinkDatum(i->fLinkInfo, this, i->fLinkRef));
  }
}

ZGlassImg::~ZGlassImg() {
  delete fFullMTW_View;
  // LinkViews should be empty ...
  if(!fLinkViews.empty()) {
    cout <<"ZGlassImg::~ZGlassImg links are NOT empty ... problems anticipated\n";
  }
  while(!fFullViews.empty()) {
    delete fFullViews.back();
    // The destructor of A_GlassView removes itself from the fFullViews list.
    // The same *must* hold for "manually" added A_View's. User's responsibility.
  }
  // delete fDefView; // this one removed in the above while
}

void ZGlassImg::CreateDefView() {
  fDefView = fIsList ? new ZListView(this) : new ZGlassView(this);
  // cout <<"ZGlassImg::CreateDefView() for "<< fGlass->GetName() << " "<< fDefView <<endl;
}

void ZGlassImg::AssertDefView() {
  if(fDefView == 0) CreateDefView();
}

/**************************************************************************/
// A_GlassView
/**************************************************************************/

A_GlassView::A_GlassView(ZGlassImg* i) : A_View(i) {
  if(fImg) fImg->CheckInFullView(this);
  fRnr = 0; fRnrScheme = 0;
}

A_GlassView::~A_GlassView() {
  // cout <<"A_GlassView::~A_GlassView of "<< fImg->fGlass->GetName() << endl;
  if(fImg) fImg->CheckOutFullView(this);
  delete fRnr; delete fRnrScheme;
}

/**************************************************************************/

void A_GlassView::SpawnRnr(const string& rnr)
{
  // cout <<"A_GlassView::SpawnRnr called for "<< fImg->fGlass->GetName() <<endl;
  fRnr = fImg->fClassInfo->fViewPart->SpawnRnr(rnr, fImg->fGlass);
}

void A_GlassView::InvalidateRnrScheme()
{
  // cout <<"A_GlassView::InvalidateRnrScheme called for "<< fImg->fGlass->GetName() <<endl;
  if(fRnrScheme) { delete fRnrScheme; fRnrScheme = 0; }
}

/**************************************************************************/
// ZGlassView
/**************************************************************************/

ZGlassView::ZGlassView(ZGlassImg* img) : A_GlassView(img) {
  for(lZLinkDatum_i l=fImg->fLinkData.begin(); l!=fImg->fLinkData.end(); ++l) {
    fLinkViews.push_back( new ZLinkView(fImg, this, &(*l)) );
  }
}

ZGlassView::~ZGlassView() {
  // cout <<"ZGlassView::~ZGlassView of "<< fImg->fGlass->GetName() << endl;
  for(lpZLinkView_i l=fLinkViews.begin(); l!=fLinkViews.end(); ++l) {
    delete *l;
  }
}

/**************************************************************************/

void ZGlassView::AssertDependantViews() {
  for(lpZLinkView_i l=fLinkViews.begin(); l!=fLinkViews.end(); ++l) {
    if((*l)->fToImg) {
      (*l)->fToImg->AssertDefView();
    }
  }
}

void ZGlassView::CopyLinkViews(lpZLinkView_t& v)
{
  copy(fLinkViews.begin(), fLinkViews.end(), back_inserter(v));
}

/**************************************************************************/


void ZGlassView::AbsorbRay(Ray& ray)
{
  // No parental absorber!

  if(ray.fRQN != RayNS::RQN_link_change)
    return;

  for(lpZLinkView_i l=fLinkViews.begin(); l!=fLinkViews.end(); ++l) {
    if((*l)->NeedsUpdate()) (*l)->Update();
  }
}

/**************************************************************************/
// ZListView
/**************************************************************************/

void ZListView::build_daughters()
{
  ZList* l = dynamic_cast<ZList*>(fImg->fGlass);
  lpZGlass_t d; l->Copy(d);
  for(lpZGlass_i i=d.begin(); i!=d.end(); ++i) {
    fDaughterImgs.push_back( fImg->fEye->DemanglePtr((*i)) );
  }
}

ZListView::ZListView(ZGlassImg* img) : ZGlassView(img)
{
  build_daughters();
}

/**************************************************************************/

void ZListView::AssertDependantViews() {
  ZGlassView::AssertDependantViews();
  for(lpZGlassImg_i i=fDaughterImgs.begin(); i!=fDaughterImgs.end(); ++i) {
    (*i)->AssertDefView();
  }
}

void ZListView::CopyListViews(lpA_GlassView_t& v)
{
  for(lpZGlassImg_i i=fDaughterImgs.begin(); i!=fDaughterImgs.end(); ++i) {
    v.push_back((*i)->fDefView);
  }
}

/**************************************************************************/

void ZListView::AbsorbRay(Ray& ray)
{
  using namespace RayNS;
  switch(ray.fRQN) {

  case RQN_list_add: {
    lpZGlassImg_i i = ray.fGammaImg ?
      find(fDaughterImgs.begin(), fDaughterImgs.end(), ray.fGammaImg) :
      fDaughterImgs.end();
    fDaughterImgs.insert(i, ray.fBetaImg);
    return;
  }

  case RQN_list_remove: {
    fDaughterImgs.remove(ray.fBetaImg);
    return;
  }

  case RQN_list_rebuild: {
    fDaughterImgs.clear();
    build_daughters();
    return;
  }

  case RQN_list_clear: {
    fDaughterImgs.clear();
    return;
  }

  } // end switch ray.fRQN

  ZGlassView::AbsorbRay(ray);
}
/**************************************************************************/

void ZListView::AssertListRnrs(const string& rnr) {
  for(lpZGlassImg_i i=fDaughterImgs.begin(); i!=fDaughterImgs.end(); ++i) {
    (*i)->fDefView->AssertRnr(rnr);
  }
}

/**************************************************************************/
// ZLinkView
/**************************************************************************/

ZLinkView::~ZLinkView() {
  if(fToImg) fToImg->CheckOutLinkView(this);
}

void ZLinkView::Update() {
  if(fToImg) fToImg->CheckOutLinkView(this);
  fToGlass = fLinkDatum->fLinkRef;
  fToImg = fToGlass ? fImg->fEye->DemanglePtr(fToGlass) : 0;
  if(fToImg) fToImg->CheckInLinkView(this);
}

