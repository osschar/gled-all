// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// RnrDriver
//
// Generic RnrDriver implementation.
// Currently still GL-only, as infrastructure for POV-Ray rendering is not
// yet prepared.

#include "RnrDriver.h"
#include <Eye/Eye.h>
#include <Glasses/ZGlass.h>

namespace OS = OptoStructs;
namespace GVNS = GledViewNS;

void RnrDriver::AssertGlassRnr(OS::A_GlassView* gv)
{
  gv->AssertRnr(mRnrName);
  /* This mostly false ...
  if(gv->fRnrCtrl->fUseOwnRnr) {
    if(gv->fRnr == 0) {
      ZGlass* d = gv->fImg->fGlass;
      gv->fRnr = GledViewNS::SpawnRnr(mRnrName, d, d->ZibID(), d->ZlassID());
    }
  } else {
    if(gv->fImg->HasRnr() == false) {
      gv->fImg->SetRnr(GledViewNS::SpawnRnr(mRnrName, d, d->ZibID(), d->ZlassID()));
    }
  }
  */
}

void RnrDriver::AssertListRnrs(OS::A_GlassView* lv)
{
  // use virtual First/Last foonctions from listview to obtain
  // appropriate GlassViews (or its sub-classes as in FTW_Leaf).
  // Need locks for multithread viewer ... terribly easy to deadlock

  // if bUseOwnRnrs .... !!!
  lv->AssertListRnrs(mRnrName);
}

/**************************************************************************/

void RnrDriver::PrepareRnrElements(OS::A_GlassView* gv, vlRnrElement_t& rev)
{
  // First all dependant views
  gv->AssertDependantViews();

  const GVNS::RnrBits& top_rb = gv->GetRnrBits();
  // Low level renderer for gv
  if(top_rb.SelfOnDirect()) {
    AssertGlassRnr(gv);
    fill_rnrelem_vec(gv, top_rb, rev, false, false);
  }

  // Low level renderers for links of gv
  if(gv->GetRnrCtrl().fRnrLinks) {
    OS::lpZLinkView_t links; gv->CopyLinkViews(links);
    for(OS::lpZLinkView_i linkv=links.begin(); linkv!=links.end(); ++linkv) {
      if((*linkv)->fToImg) {
	const GVNS::RnrBits& rb	 = (*linkv)->GetRnrBits();
	OS::A_GlassView* to_view = (*linkv)->GetView();

	if(rb.SelfOnDirect()) {
	  AssertGlassRnr(to_view);
	  fill_rnrelem_vec(to_view, rb, rev, false, true);
	}

	if((*linkv)->fToImg->fIsList && rb.ListOn()) {
	  to_view->AssertDependantViews();
	  if(rb.ListOnDirect()) AssertListRnrs(to_view);
	  fill_rnrelem_vec(to_view, rb, rev, true, true);
	}
      }
    }
  }

  // Low level renderers for list members of gv
  if(gv->fImg->fIsList && top_rb.ListOn()) {
    if(top_rb.ListOnDirect()) {
      AssertListRnrs(gv);
    }
    fill_rnrelem_vec(gv, top_rb, rev, true, true);
  }
  
}

/**************************************************************************/

void RnrDriver::Render(OS::A_GlassView* gv)
{

  UChar_t max_rnr_level = gv->GetRnrCtrl().fMaxLvl;
  if(max_rnr_level == 0) return;
  if(mMaxDepth <= 0) return;
  --mMaxDepth;
  
  if(gv->fRnrScheme == 0) {
    gv->fRnrScheme = new RnrScheme(max_rnr_level + 1);
    //***
    //cout <<"RnrDriver::Render making RnrScheme for "
    //<< gv->fImg->fGlass->GetName() <<endl;
    PrepareRnrElements(gv, gv->fRnrScheme->fScheme);
  }
    
  for(UChar_t rl=1; rl<=max_rnr_level; ++rl) {
    lRnrElement_t& re_list = gv->fRnrScheme->fScheme[rl];
    for(lRnrElement_i re=re_list.begin(); re!=re_list.end(); ++re) {
      if(re->fView) {
	//***
	//cout <<"RnrDriver::Render Rendering "
        //<< re->fView->fImg->fGlass->GetName() <<endl;
	Render(re->fView);
      } else {
	re->fRnr->GetGlass()->ReadLock();
	((re->fRnr)->*(re->fRnrFoo))(this);
	re->fRnr->GetGlass()->ReadUnlock();
      }
    }
  }

  ++mMaxDepth;
}

/**************************************************************************/


void RnrDriver::fill_rnrelem_vec(OS::A_GlassView* gv, const GVNS::RnrBits& bits,
				 vlRnrElement_t& rev,
				 bool as_list, bool full_descent)
{
  if(as_list) {

    OS::lpA_GlassView_t el_views; gv->CopyListViews(el_views);
    if(bits.fList[0]) {
      for(OS::lpA_GlassView_i v=el_views.begin(); v!=el_views.end(); ++v)
	rev[bits.fList[0]].push_back(RnrElement(gv->GetRnr(mRnrName), &A_Rnr::PreDraw));
    }
    if(bits.fList[1]) {
      for(OS::lpA_GlassView_i v=el_views.begin(); v!=el_views.end(); ++v)
	rev[bits.fList[1]].push_back(RnrElement(gv->GetRnr(mRnrName), &A_Rnr::Draw));
    }
    if(bits.fList[2]) {
      for(OS::lpA_GlassView_i v=el_views.begin(); v!=el_views.end(); ++v)
	rev[bits.fList[2]].push_back(RnrElement(gv->GetRnr(mRnrName), &A_Rnr::PostDraw));
    }
    if(full_descent && bits.fList[3]) {
      for(OS::lpA_GlassView_i v=el_views.begin(); v!=el_views.end(); ++v)
	rev[bits.fList[3]].push_back(RnrElement(*v));
    }

  } else {

    if(bits.fSelf[0]) {
      rev[bits.fSelf[0]].push_back(RnrElement(gv->GetRnr(mRnrName), &A_Rnr::PreDraw));
    }
    if(bits.fSelf[1]) {
      rev[bits.fSelf[1]].push_back(RnrElement(gv->GetRnr(mRnrName), &A_Rnr::Draw));
    }
    if(bits.fSelf[2]) {
      rev[bits.fSelf[2]].push_back(RnrElement(gv->GetRnr(mRnrName), &A_Rnr::PostDraw));
    }
    if(full_descent && bits.fSelf[3]) {
      rev[bits.fSelf[3]].push_back(RnrElement(gv));
    }

  }
}

/**************************************************************************/
/**************************************************************************/

void RnrDriver::InitLamps()
{
  for(int i=0; i<mMaxLamps; ++i) {
    mLamps[i] = 0;
  }
}

int RnrDriver::GetLamp(A_Rnr* l_rnr)
{
  for(int i=0; i<mMaxLamps; ++i) {
    if(mLamps[i] == 0) { mLamps[i] = l_rnr; return i; }
  }
  return -1;
}

void RnrDriver::ReturnLamp(int lamp)
{
  mLamps[lamp] = 0;
}

/**************************************************************************/

A_Rnr* RnrDriver::GetDefRnr(ZGlass* g)
{
  OS::ZGlassImg* img = mEye->DemanglePtr(g);
  img->AssertDefView();
  img->fDefView->AssertRnr(mRnrName);
  return img->fDefView->GetRnr(mRnrName);
}

/**************************************************************************/
