// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "FTW_Branch.h"
#include "FTW_Nest.h"
#include "FTW_Shell.h"

#include <Eye/Eye.h>
#include <Glasses/AList.h>
#include <Stones/ZMIR.h>

#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Value_Input.H>

namespace OS = OptoStructs;
using namespace FTW;

typedef list<FTW_Leaf*>::iterator leaf_iterator;

/**************************************************************************/

namespace {

void cb_list_collexp(Fl_Button* b, FTW_Branch* l)
{ l->CollExpList(); }

void cb_label_change(Fl_Input* w, FTW_Leaf* l)
{
  if(strcmp(l->GetListLabel(), w->value()) == 0)
    return;
  AList* list = l->GetParent()->fImg->GetList();
  auto_ptr<ZMIR> mir
    (list->MkMir_ChangeLabel(l->GetListLabel(), w->value()));
  l->GetParent()->fImg->fEye->Send(*mir);
}

}

/**************************************************************************/

void FTW_Branch::build_leaves(int insert_pos)
{
  AList* list = (AList*) fImg->fLens;
  bool has_zeroes = list->elrep_can_hold_zero();
  AList::lElRep_t d;
  mLeavesTimeStamp = list->CopyListElReps(d, has_zeroes);
  for(AList::lElRep_i i=d.begin(); i!=d.end(); ++i) {
    OS::ZGlassImg* img = fImg->fEye->DemanglePtr(i->get_lens());
    FTW_Leaf*     leaf = create_leaf(img, *i);
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

FTW_Leaf* FTW_Branch::create_leaf(OptoStructs::ZGlassImg* img,
                                  const AList::ElRep& el_rep)
{
  FTW_Leaf* leaf = FTW_Leaf::Construct(mNest, this, img, true, false);
  leaf->mListId    = el_rep.get_id();
  leaf->mListLabel = el_rep.ref_label();
  if (leaf->wListDesignation != 0) {
    leaf->label_designation();
    leaf->wListDesignation->callback((Fl_Callback*) cb_label_change, leaf);
    leaf->wListDesignation->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY);
  }
  return leaf;
}

void FTW_Branch::insert_leaf(Ray& ray, lpFTW_Leaf_i pos)
{
  int nest_pos = (pos == mLeaves.end()) ?
    mNest->PackPosAfter(this) : mNest->PackPos(*pos);
  FTW_Leaf* leaf = create_leaf(ray.fBetaImg, beta_elrep(ray));
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

FTW_Branch::lpFTW_Leaf_i FTW_Branch::leaf_pos(AList::ElRep elrep)
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
    if(elrep.ref_label().IsNull())
      return mLeaves.end();
    else
      return find_if(mLeaves.begin(), mLeaves.end(),
                     FTW_Leaf::leaf_eq_label(elrep.ref_label()));
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
  bLeavesCreated(false), bListExpanded(false),
  mLDShow(false), mLDEdit(false), mLDWidth(6), mLDNameMinWidth(8)
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
	remove_leaf(--mLeaves.end());
	break;
      case RQN_list_push_front:
	insert_leaf(ray, mLeaves.begin());
	break;
      case RQN_list_pop_front:
	remove_leaf(mLeaves.begin());
	break;

      case RQN_list_insert: 
	insert_leaf(ray, leaf_pos(gamma_elrep(ray)));
	break;
      case RQN_list_remove:
	remove_leaf(leaf_pos(beta_elrep(ray)));
	break;

      case RQN_list_insert_label:
        insert_leaf(ray, leaf_pos(gamma_elrep(ray)));
        break;

      case RQN_list_remove_label:
        remove_leaf(leaf_pos(beta_elrep(ray)));
	break;

      case RQN_list_element_set: {
        lpFTW_Leaf_i pos = leaf_pos(beta_elrep(ray));
        insert_leaf(ray, pos);
        remove_leaf(pos);
	break;
      }

      case RQN_list_rebuild:
      case RQN_list_clear: {
	bool was_expanded = bListExpanded;
	if(bListExpanded) CollapseList();
	wipe_leaves();
	if(was_expanded)  ExpandList();
        break;
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

void FTW_Branch::RewidthNameBoxes()
{
  for(leaf_iterator i=mLeaves.begin(); i!=mLeaves.end(); ++i) {
    (*i)->resize_weeds(); // !!! should only resize namebox/designator
  }
  mNest->GetPack()->redraw();
}

namespace {
class LeafDesignationCtrl : public Fl_Window
{
public:
  FTW_Branch*      fBranch;
  FTW::NameBox*    fNameBox;
  Fl_Check_Button* fShow;
  Fl_Check_Button* fEdit;
  Fl_Value_Input*  fWidth;
  Fl_Value_Input*  fNameMinWidth;

  LeafDesignationCtrl() : Fl_Window(16, 5, "Leaf Designations")
  {
    fBranch = 0;
    int y = 0;
    fNameBox = new FTW::NameBox(0, y, 0, 16, 1);
    ++y;
    fShow = new Fl_Check_Button(0, y, 8, 1, "Show");
    fEdit = new Fl_Check_Button(8, y, 8, 1, "Edit");
    ++y;
    fWidth = new Fl_Value_Input(12, y, 4, 1, "DesignationWidth");
    fWidth->range(2, 20); fWidth->step(1);
    ++y;
    fNameMinWidth = new Fl_Value_Input(12, y, 4, 1, "NameMinWidth");
    fNameMinWidth->range(4, 30); fNameMinWidth->step(1);
    ++y;
    Fl_Button* b;
    b = new Fl_Button(0, y, 8, 1, "Cancel");
    b->callback((Fl_Callback*)cancel_cb, this);
    b = new Fl_Return_Button(8, y, 8, 1, "OK");
    b->callback((Fl_Callback*)ok_cb, this);
  }

  void set_branch(FTW_Branch* b) {
    fBranch = b;
    fNameBox->ChangeImage(b->fImg);
    fShow->value(b->GetLDShow());
    fEdit->value(b->GetLDEdit());
    if(fBranch->fImg->GetList()->elrep_can_edit_label())
      fEdit->activate();
    else
      fEdit->deactivate();
    fWidth->value(b->GetLDWidth());
    fNameMinWidth->value(b->GetLDNameMinWidth());
  }
  void set_values() {
    fBranch->SetLDShow(fShow->value());
    fBranch->SetLDEdit(fEdit->value());
    fBranch->SetLDWidth((int) fWidth->value());
    fBranch->SetLDNameMinWidth((int) fNameMinWidth->value());
    fBranch->RewidthNameBoxes();
  }

  static void ok_cb(Fl_Button* w, LeafDesignationCtrl* c) {
    c->set_values();
    cancel_cb(w, c);
  }
  static void cancel_cb(Fl_Button* w, LeafDesignationCtrl* c) {
    c->hide();
    c->fNameBox->ChangeImage(0);
    c->fBranch = 0;
  }
};

LeafDesignationCtrl* gLeafDesignationCtrl = 0;
}

void FTW_Branch::SpawnSeparatorDialog()
{
  if(gLeafDesignationCtrl == 0) {
    gLeafDesignationCtrl = new LeafDesignationCtrl();
    gLeafDesignationCtrl->set_non_modal();
    mNest->GetShell()->adopt_window(gLeafDesignationCtrl);
    
  }
  gLeafDesignationCtrl->set_branch(this);
  gLeafDesignationCtrl->hotspot(gLeafDesignationCtrl);
  gLeafDesignationCtrl->show();
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

void FTW_Branch::label_namebox()
{
  int s = bLeavesCreated ? mLeaves.size() : ((AList*)fImg->fLens)->Size();
  if(s) {
    wName->set_label(GForm("%s [%d]", fImg->fLens->GetName(), s));
  } else {
    wName->set_label(GForm("%s []", fImg->fLens->GetName()));
  }
}

void FTW_Branch::label_weeds()
{
  if(bListExpanded) {
    wListExpander->label("@#-2>");
  } else {
    wListExpander->label("@#-2>|");
  }
  wListExpander->redraw_label();
  FTW_Leaf::label_weeds();
}
