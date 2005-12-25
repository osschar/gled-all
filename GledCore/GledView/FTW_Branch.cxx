// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "FTW_Branch.h"
#include "FTW_Nest.h"
#include "FTW_Shell.h"

#include <Eye/Eye.h>
#include <Glasses/ZList.h>

#include <FL/Fl_Button.H>

namespace OS = OptoStructs;
using namespace FTW;

typedef list<FTW_Leaf*>::iterator leaf_iterator;

/**************************************************************************/

static void cb_list_collexp(Fl_Button* b, FTW_Branch* l) {
  l->CollExpList();
}

/**************************************************************************/

void FTW_Branch::build_leaves(int insert_pos)
{
  AList::lElRep_t d;
  mLeavesTimeStamp = ((AList*)fImg->fLens)->CopyListElReps(d);
  for(AList::lElRep_i i=d.begin(); i!=d.end(); ++i) {
    OS::ZGlassImg* img = fImg->fEye->DemanglePtr(i->get_lens());
    FTW_Leaf*     leaf = FTW_Leaf::Construct(mNest, this, img, true, false);
    leaf->mListId    = i->get_id();
    leaf->mListLabel = i->get_label();
    mLeaves.push_back(leaf);
    mNest->InsertLeaf(leaf, insert_pos++);
  }
}

void FTW_Branch::wipe_leaves()
{
  if(bLeavesCreated) {
    for(leaf_iterator i=mLeaves.begin(); i!=mLeaves.end(); ++i)
      delete *i;
    bLeavesCreated = false;
  }
  mLeaves.clear();
}

/**************************************************************************/

void FTW_Branch::insert_leaf(Ray& ray, list<FTW_Leaf*>::iterator pos)
{
  int nest_pos = (pos == mLeaves.end()) ?
    mNest->PackPosAfter(this) : mNest->PackPos(*pos);
  FTW_Leaf* leaf = FTW_Leaf::Construct(mNest, this, ray.fBetaImg, true, false);
  leaf->mListId    = ray.fBetaId;
  leaf->mListLabel = ray.fBetaLabel;
  mLeaves.insert(pos, leaf);
  mNest->InsertLeaf(leaf, nest_pos);
}

void FTW_Branch::remove_leaf(list<FTW_Leaf*>::iterator pos)
{
  static const Exc_t _eh("FTW_Branch::remove_leaf ");

  if(pos == mLeaves.end()) {
    mNest->GetShell()->Message
      (_eh + "got end() as position argument. Expect trouble.");
      return;
  }
  delete *pos;
  mLeaves.erase(pos);
}

/**************************************************************************/

AList::ElRep FTW_Branch::beta_elrep(Ray& ray)
{
  return AList::ElRep(ray.fBeta, ray.fBetaId, ray.fBetaLabel);
}

AList::ElRep FTW_Branch::gamma_elrep(Ray& ray)
{
  return AList::ElRep(ray.fGamma, ray.fGammaId, ray.fGammaLabel);
}

list<FTW_Leaf*>::iterator FTW_Branch::leaf_pos(AList::ElRep elrep)
{
  AList::ElType_e type = ((AList*)fImg->fLens)->el_type();
  switch(type) {
  case AList::ET_Lens:
    return find_if(mLeaves.begin(), mLeaves.end(),
		   FTW_Leaf::leaf_eq_lens(elrep.get_lens()));
  case AList::ET_Id:
    return find_if(mLeaves.begin(), mLeaves.end(),
		   FTW_Leaf::leaf_eq_id(elrep.get_id()));
  case AList::ET_Label:
    return find_if(mLeaves.begin(), mLeaves.end(),
		   FTW_Leaf::leaf_eq_label(elrep.get_label()));
  default:
    return mLeaves.end();
  }
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
}

FTW_Branch::~FTW_Branch() {
  wipe_leaves();
}

/**************************************************************************/

void FTW_Branch::AbsorbRay(Ray& ray)
{
  using namespace RayNS;

  if(ray.fRQN > RQN_list_begin && ray.fRQN < RQN_list_end) {

    if(bLeavesCreated && mLeavesTimeStamp < ray.fStamp) {

      switch(ray.fRQN) {

      case RQN_list_push_back:
	insert_leaf(ray, mLeaves.end());
	break;
      case RQN_list_pop_back:
	// delete mLeaves.back();
	remove_leaf(--mLeaves.end());
	break;
      case RQN_list_push_front:
	insert_leaf(ray, mLeaves.begin());
	break;
      case RQN_list_pop_front:
	//delete mLeaves.front();
	remove_leaf(mLeaves.begin());
	break;

      case RQN_list_insert: 
	insert_leaf(ray, leaf_pos(gamma_elrep(ray)));
	break;
      case RQN_list_remove:
	remove_leaf(leaf_pos(beta_elrep(ray)));
	break;

      case RQN_list_element_set:
      case RQN_list_insert_label:
      case RQN_list_remove_label:
	cout << "Branch::AbsorbRay "<< ray << ". Not supported yet.\n";
	break;
	//case RQN_list_rebuild:
	//      case RQN_list_clear:

	/*
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
	//cout <<"FTW_Branch::AbsorbRay RQN_list_remove for "<< eximg->fLens->GetName() <<
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
	*/

      case RQN_list_rebuild:

      case RQN_list_clear: {
	bool was_expanded = bListExpanded;
	if(bListExpanded) CollapseList();
	wipe_leaves();
	if(was_expanded)  ExpandList();
	return;
      }

      } // end switch ray.fRQN

    }

    label_namebox();

  } // enf if list-ray

  FTW_Leaf::AbsorbRay(ray);
}

/**************************************************************************/

void FTW_Branch::CollExpList()
{
  if(bListExpanded) CollapseList(); else ExpandList();
}

void FTW_Branch::ExpandList() {
  static const Exc_t _eh("FTW_Branch::ExpandList ");

  if(bListExpanded) return; // ants can call this for no good reason
  if(!bLeavesCreated) {
    if(((AList*)fImg->fLens)->Size() > 128000) {
      mNest->GetShell()->Message
	(_eh + "refusing to expand list longer than 128000 elements.", Eye::MT_wrn);
      return;
    }
    build_leaves( mNest->PackPosAfter(this) );
    bLeavesCreated = true;
  } else {
    for(leaf_iterator i=mLeaves.begin(); i!=mLeaves.end(); ++i)
      (*i)->ShowRecursively();
  }
  bListExpanded = true;
  label_weeds();
  mNest->redraw();
}

void FTW_Branch::CollapseList()
{
  if(!bListExpanded) return;
  for(leaf_iterator i=mLeaves.begin(); i!=mLeaves.end(); ++i)
    (*i)->HideRecursively();
  bListExpanded = false;
  label_weeds();
  mNest->redraw();
}

void FTW_Branch::CopyListLeaves(list<FTW_Leaf*>& list)
{
  if(!bLeavesCreated) return;
  copy(mLeaves.begin(), mLeaves.end(), back_inserter(list));
}

/**************************************************************************/

void FTW_Branch::ShowListMembers()
{
  if(bListExpanded)
    for(leaf_iterator i=mLeaves.begin(); i!=mLeaves.end(); ++i)
      (*i)->ShowRecursively();
}

void FTW_Branch::HideListMembers()
{
  if(bListExpanded)
    for(leaf_iterator i=mLeaves.begin(); i!=mLeaves.end(); ++i)
      (*i)->HideRecursively();
}

/**************************************************************************/

float FTW_Branch::LeafPosition(FTW_Leaf* leaf)
{
  if(!bLeavesCreated) return 0;
  int j = 0;
  for(leaf_iterator i=mLeaves.begin(); i!=mLeaves.end(); ++i)
    if(*i == leaf) return float(j)/mLeaves.size();
  return 1;
}

/**************************************************************************/
/**************************************************************************/
// protected
/**************************************************************************/
/**************************************************************************/

void FTW_Branch::label_namebox() {
  int s = bLeavesCreated ? mLeaves.size() : ((AList*)fImg->fLens)->Size();
  if(s) {
    wName->set_label(GForm("%s [%d]", fImg->fLens->GetName(), s));
  } else {
    wName->set_label(GForm("%s []", fImg->fLens->GetName()));
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
