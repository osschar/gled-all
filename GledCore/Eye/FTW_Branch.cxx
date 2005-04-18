// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "FTW_Branch.h"
#include "FTW_Nest.h"
#include "FTW_Shell.h"
#include "Eye.h"
#include <Glasses/ZList.h>

#include <FL/Fl_Button.H>

namespace OS = OptoStructs;
using namespace FTW;

/**************************************************************************/

static void cb_list_collexp(Fl_Button* b, FTW_Branch* l) {
  l->CollExpList();
}

void FTW_Branch::build_leoim()
{
  ZList* l = dynamic_cast<ZList*>(fImg->fGlass);
  lpZGlass_t d; mLeoimTimeStamp = l->Copy(d);
  for(lpZGlass_i i=d.begin(); i!=d.end(); ++i) {
    mLeoim.push_back(Leaf_o_Img(fImg->fEye->DemanglePtr(*i)));
  }
}

void FTW_Branch::wipe_leoim()
{
  if(bLeavesCreated) {
    for(lLoI_i i=mLeoim.begin(); i!=mLeoim.end(); ++i)
      delete i->leaf;
    bLeavesCreated = false;
  }
  mLeoim.clear();
}

/**************************************************************************/
// FTW_Branch
/**************************************************************************/

FTW_Branch::FTW_Branch(FTW_Nest* nest, FTW_Leaf* parent,
		       OS::ZGlassImg* img,
		       bool is_list_member, bool is_link_desc) :
  FTW_Leaf(nest, parent, img, is_list_member, is_link_desc),
  bLeavesCreated(false), bListExpanded(false)
{
  wListExpander->labelcolor(FL_BLACK);
  wListExpander->callback((Fl_Callback*)cb_list_collexp, this);

  //build_leoim();
}

FTW_Branch::~FTW_Branch() {
  wipe_leoim();
}

/**************************************************************************/

void FTW_Branch::AbsorbRay(Ray& ray)
{
  using namespace RayNS;

  if(ray.fRQN > RQN_list_begin && ray.fRQN < RQN_list_end) {

    if(bLeavesCreated && mLeoimTimeStamp < ray.fStamp) {

      switch(ray.fRQN) {

      case RQN_list_add: {
	lLoI_i i = ray.fGammaImg ?
	  find_if(mLeoim.begin(), mLeoim.end(), Leoim_img_eq(ray.fGammaImg)) :
	  mLeoim.end();

	FTW_Leaf* leaf = FTW_Leaf::Construct(mNest, this, ray.fBetaImg, true, false);
	if(!bListExpanded) leaf->hide();
	if(i==mLeoim.end()) mNest->InsertLeaf(leaf, mNest->PackPosAfter(this));
	else                mNest->InsertLeaf(leaf, i->leaf);

	mLeoim.insert(i, Leaf_o_Img(ray.fBetaImg, leaf));

	label_namebox();
	return;
      }

      case RQN_list_remove: {
	lLoI_i i = find_if(mLeoim.begin(), mLeoim.end(), Leoim_img_eq(ray.fBetaImg));
	//cout <<"FTW_Branch::AbsorbRay RQN_list_remove for "<< eximg->fGlass->GetName() <<
	//", leaf="<< i->leaf <<endl;
	if(i == mLeoim.end()) {
	  cout <<"FTW_Branch::AbsorbRay RQN_list_remove didn't find view to remove ...\n";
	  return;
	}
	// mNest->RemoveLeaf(i->leaf); // Called in Leaf dtor.
	delete i->leaf;

	mLeoim.erase(i);
	label_namebox();
	return;
      }

      case RQN_list_rebuild: {
	bool was_expanded = bListExpanded;
	if(bListExpanded) CollapseList();
	wipe_leoim();
	build_leoim();
	if(was_expanded)  ExpandList();
	label_namebox();
	return;
      }

      case RQN_list_clear: {
	bool was_expanded = bListExpanded;
	if(bListExpanded) CollapseList();
	wipe_leoim();
	if(was_expanded)  ExpandList();
	label_namebox();
	return;
      }

      } // end switch ray.fRQN

    } else { // leaves not created

      label_namebox();

    }

  } // enf if list-ray

  FTW_Leaf::AbsorbRay(ray);
}

/**************************************************************************/

void FTW_Branch::CollExpList()
{
  if(bListExpanded) CollapseList(); else ExpandList();
}

void FTW_Branch::ExpandList() {
  static const string _eh("FTW_Branch::ExpandList ");

  if(bListExpanded) return; // ants can call this for no good reason
  if(!bLeavesCreated) {
    if(((ZList*)fImg->fGlass)->Size() > 1024) {
      mNest->GetShell()->Message
	(_eh + "refusing to expand list longer than 1024 elements.", Eye::MT_wrn);
      return;
    }

    int before_pos = mNest->PackPosAfter(this);
    build_leoim();
    for(lLoI_i i=mLeoim.begin(); i!=mLeoim.end(); ++i) {
      i->leaf = FTW_Leaf::Construct(mNest, this, i->img, true, false);
      mNest->InsertLeaf(i->leaf, before_pos++);
    }
    bLeavesCreated = true;
  } else {
    for(lLoI_i i=mLeoim.begin(); i!=mLeoim.end(); ++i) {
      i->leaf->ShowRecursively();
    }
  }
  bListExpanded = true;
  label_weeds();
  mNest->redraw();
}

void FTW_Branch::CollapseList()
{
  if(!bListExpanded) return;
  for(lLoI_i i=mLeoim.begin(); i!=mLeoim.end(); ++i) {
    i->leaf->HideRecursively();
  }
  bListExpanded = false;
  label_weeds();
  mNest->redraw();
}

void FTW_Branch::CopyListLeaves(list<FTW_Leaf*>& list)
{
  if(!bLeavesCreated) return;
  for(lLoI_i i=mLeoim.begin(); i!=mLeoim.end(); ++i) {
    list.push_back(i->leaf);
  } 
}

/**************************************************************************/

void FTW_Branch::ShowListMembers()
{
  if(bListExpanded)
    for(lLoI_i i=mLeoim.begin(); i!=mLeoim.end(); ++i)
      i->leaf->ShowRecursively();
}

void FTW_Branch::HideListMembers()
{
  if(bListExpanded)
    for(lLoI_i i=mLeoim.begin(); i!=mLeoim.end(); ++i)
      i->leaf->HideRecursively();
}

/**************************************************************************/

float FTW_Branch::LeafPosition(FTW_Leaf* leaf)
{
  if(!bLeavesCreated) return 0;
  int j;
  for(lLoI_i i=mLeoim.begin(); i!=mLeoim.end(); ++i, ++j) {
    if(i->leaf == leaf) return float(j)/mLeoim.size();
  }
  return 1;
}

/**************************************************************************/
/**************************************************************************/
// protected
/**************************************************************************/
/**************************************************************************/

void FTW_Branch::label_namebox() {
  int s = bLeavesCreated ? mLeoim.size() : ((ZList*)fImg->fGlass)->Size();
  if(s) {
    wName->set_label(GForm("%s [%d]", fImg->fGlass->GetName(), s));
  } else {
    wName->set_label(GForm("%s []", fImg->fGlass->GetName()));
  }
}

void FTW_Branch::label_weeds() {
  if(bListExpanded) {
    wListExpander->label("@#-2>");
  } else {
    wListExpander->label("@#-2>|");
  }
  wListExpander->redraw_label();
  FTW_Leaf::label_weeds();
}
