// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "FTW_Leaf.h"
#include "FTW_Ant.h"
#include "FTW_Branch.h"
#include "FTW_Nest.h"
#include "FTW_Shell.h"
#include "FltkGledStuff.h"
#include <Glasses/ZGlass.h>
#include <Glasses/NestInfo.h>
#include <Eye/Ray.h>

#include "MTW_ClassView.h"
#include "MTW_Layout.h"

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Output.H>

namespace OS  = OptoStructs;
namespace FGS = FltkGledStuff;

/**************************************************************************/

namespace {

void cb_collex(Fl_Button* b, FTW_Leaf* l)
{ l->CollExp(); }

void cb_separator_dialog(Fl_Button* sep, FTW_Leaf* l)
{ l->SpawnSeparatorDialog(); }

}

/**************************************************************************/
// FTW_Leaf
//
// One line of FTW_Nest.
// Provides functionality for displaying links and custom MTW_Views.
//
// Interface for displaying subclasses of ZList also provided, but methods
// are virtual and trivially implemented. Overriden in FTW_Branch : pub FTW_Leaf.
//
// FTW_Leaf Navigation for custom views. #^^^#
// Keyboard navigation is broken as standard focus handling methods of
// Fl_Group are used. Should redefine Fl_Group::handle(int ev) and implement
// special management of navigation keys. (see  Fl_Group::handle / navigation).
// The idea would be to, for Up/Down events, check Fl::focus() and try to cast
// its parent's parent (...?) to FTW_Leaf. Then pass focus to appropriate
// widget in this leaf.
// The problem is, that FTW_Nest's display is line-oriented and, say, up-key
// jumps to the *last* widget in previous line.
/**************************************************************************/

/* static */
FTW_Leaf*
FTW_Leaf::Construct(FTW_Nest* nest, FTW_Leaf* parent,
		    OS::ZGlassImg* img,
		    bool is_list_member, bool is_link_desc)
{
  FTW_Leaf* l;
  if(img && img->fIsList)
    l = new FTW_Branch(nest, parent, img, is_list_member, is_link_desc);
  else
    l = new FTW_Leaf(nest, parent, img, is_list_member, is_link_desc);
  l->label_weeds();
  l->label_namebox();
  return l;
}

FTW_Leaf::FTW_Leaf(FTW_Nest* nest, FTW_Leaf* parent,
		   OS::ZGlassImg* img,
		   bool is_list_member, bool is_link_desc) :
  Fl_Pack(0,0,1,1),
  A_View(img),
  mNest(nest), mParent(parent),
  bIsListMember(is_list_member), bIsLinkDescendant(is_link_desc)
{
  mIter   = -1;
  mLevel  = mParent ? mParent->GetLevel() + 1 : 0;
  mListId = -1;

  type(FL_HORIZONTAL);

  wFrontPack = new Fl_Pack(0,0,1,1); wFrontPack->type(FL_HORIZONTAL);
  wIndentBox = new Fl_Box(0,0,1,1);
  wIndentBox->box(FL_FLAT_BOX);

  wExpander  = new Fl_Button(0,0,1,1);
  wExpander->color(FTW::background_color);
  wExpander->labeltype(FL_SYMBOL_LABEL);
  wExpander->callback((Fl_Callback*)cb_collex, this);

  wListExpander = new Fl_Button(0,0,1,1,"@#-2>|");
  wListExpander->color(FTW::background_color);
  wListExpander->labeltype(FL_SYMBOL_LABEL);
  wListExpander->labelcolor(FL_DARK_RED);

  wName = new FTW::NameButton(this, 0,0,1,1);
  wName->color(FTW::background_color);
  wName->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);

  wListDesignation = 0;
  if(bIsListMember) {
    AList* l = mParent->fImg->GetList();
    AList::ElType_e eltype = l->el_type();
    if(eltype == AList::ET_Id || eltype == AList::ET_Label)
      wListDesignation = new FTW::ListDesignator(l->elrep_can_edit_label());
  }

  wSepBox = new Fl_Button(0,0,1,1,0);
  if(fImg && fImg->fIsList && fImg->GetList()->el_type() >= AList::ET_Id) {
    wSepBox->box(FTW::postnamebox_box);
    if(fImg->GetList()->elrep_can_edit_label())
      wSepBox->color(FTW::postnamebox_edit_color);
    else
      wSepBox->color(FTW::postnamebox_color);
    wSepBox->callback((Fl_Callback*)cb_separator_dialog, this);
  } else {
    wSepBox->box(FTW::separator_box);
    wSepBox->down_box(FTW::separator_box);
    wSepBox->color(FTW::separator_color, FTW::separator_color);
  }

  wFrontPack->end();

  wAntPack = new Fl_Pack(0,0,1,1); wAntPack->type(FL_HORIZONTAL);
  if(fImg) {
    for(OS::lZLinkDatum_i l=fImg->fLinkData.begin(); l!=fImg->fLinkData.end(); ++l) {
      new FTW_Ant(&(*l), this);
    }
  }
  wAntPack->end();

  wTailBox = new Fl_Box((Fl_Boxtype)FL_FLAT_BOX,0,0,1,1,0);

  wCustomView = 0;

  end();
  if(mNest->GetAntsReversed()) reverse_ants();
  resize_weeds();
}

FTW_Leaf::~FTW_Leaf() {
  // hrrmph ... what will be done by fltk? ants are my children ...
  // but do they have virtual destructors? yesss ... ok ... anyways
  // would be more hygienic to destruct leaves of ants here as they were
  // also created here. Koo-roomph ...
  mNest->RemoveLeaf(this);
}

/**************************************************************************/

void FTW_Leaf::AbsorbRay(Ray& ray)
{
  // No parental absorber!

  using namespace RayNS;
  switch(ray.fRQN) {

  case RQN_change: {
    assert(mIter==-1);
    if(ray.IsBasic()) {
      label_namebox();
    }
    return;
  }

  case RQN_link_change: {
    assert(mIter==-1);
    for(int c=0; c<wAntPack->children(); ++c) {
      FTW_Ant* a = dynamic_cast<FTW_Ant*>( wAntPack->child(c) );
      if(a->NeedsUpdate()) a->Update();
    }
    return;
  }

  } // end switch ray.fRQN
}

/**************************************************************************/
// Expand / Collapse mechanism
/**************************************************************************/

void FTW_Leaf::ExpandLink(FTW_Ant* ant) {
  OS::ZGlassImg* to_img = ant->GetToImg();
  if(ant->bExpanded || to_img==0) return;
  if(ant->mLeaf == 0) {
    ant->mLeaf = FTW_Leaf::Construct(mNest, this, to_img, false, true);
    ant->mLeaf->wIndentBox->label(ant->GetLinkInfo()->fName.Data());
    ant->mLeaf->wIndentBox->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
    ant->mLeaf->wIndentBox->labelsize(ant->mLeaf->wIndentBox->labelsize() - 4);
    FTW_Leaf* after = 0;
    bool reverse = mNest->GetAntsReversed();
    int  n_children = wAntPack->children();
    for(int c=0; c<n_children; ++c) {
      FTW_Ant* a = dynamic_cast<FTW_Ant*>( wAntPack->child(reverse ? n_children-c-1 : c) );
      if(a == ant) break;
      if(a->bExpanded) after = a->mLeaf;
    }
    int before = after ? mNest->PackPosAfter(after) : mNest->PackPos(this) + 1;
    mNest->InsertLeaf(ant->mLeaf, before);
  } else {
    ant->mLeaf->ShowRecursively();
  }
  ant->bExpanded = true; ant->label_weeds();
  label_weeds();
  mNest->redraw();
}

void FTW_Leaf::CollapseLink(FTW_Ant* ant) {
  if(!ant->bExpanded) return;
  ant->mLeaf->HideRecursively();
  ant->bExpanded = false; ant->label_weeds();
  label_weeds();
  mNest->redraw();
}

void FTW_Leaf::ExpandLinks() {
  bool reverse = mNest->GetAntsReversed();
  int  n_children = wAntPack->children();
  for(int c=0; c<n_children; ++c) {
    FTW_Ant* a = dynamic_cast<FTW_Ant*>( wAntPack->child(reverse ? n_children-c-1  : c) );
    if(a->GetToImg() && ! a->bExpanded) ExpandLink(a);
  }
  label_weeds();
}

void FTW_Leaf::CollapseLinks() {
  for(int c=0; c<wAntPack->children(); ++c) {
    FTW_Ant* a = dynamic_cast<FTW_Ant*>( wAntPack->child(c) );
    if(a->GetToImg() && a->bExpanded) CollapseLink(a);
  }
  label_weeds();
}

void FTW_Leaf::CollExp() {
  if(FractionOfExpandedAnts() <= 0.5)	ExpandLinks();
  else					CollapseLinks();
}

float FTW_Leaf::FractionOfExpandedAnts() {
  if(wAntPack->children() == 0) return 0;
  float n_expanded=0, n_nonnull=0;
  for(int c=0; c<wAntPack->children(); ++c) {
    FTW_Ant* a = dynamic_cast<FTW_Ant*>( wAntPack->child(c) );
    if(a->GetToImg()) ++n_nonnull;
    if(a->bExpanded)  ++n_expanded;
  }
  return n_expanded / n_nonnull;
}

/**************************************************************************/

void FTW_Leaf::ShowRecursively() {
  show();
  ShowLinks();
  ShowListMembers();
}

void FTW_Leaf::HideRecursively() {
  hide();
  HideLinks();
  HideListMembers();
}

void FTW_Leaf::ShowLinks() {
  for(int c=0; c<wAntPack->children(); ++c) {
    FTW_Ant* a = dynamic_cast<FTW_Ant*>( wAntPack->child(c) );
    if(a->bExpanded) a->mLeaf->ShowRecursively();
  }
}

void FTW_Leaf::HideLinks() {
  for(int c=0; c<wAntPack->children(); ++c) {
    FTW_Ant* a = dynamic_cast<FTW_Ant*>( wAntPack->child(c) );
    if(a->bExpanded) a->mLeaf->HideRecursively();
  }
}

/**************************************************************************/

int FTW_Leaf::AntPos(FTW_Ant* a) { return wAntPack->find(a); }

FTW_Ant* FTW_Leaf::AntAt(int i) {
  if(i >= wAntPack->children()) return 0;
  return dynamic_cast<FTW_Ant*>(wAntPack->child(i));
}

FTW_Ant* FTW_Leaf::LastAnt() {
  return (wAntPack->children() >= 0) ?
    dynamic_cast<FTW_Ant*>(wAntPack->child(wAntPack->children() - 1)) :
    0;
}

/**************************************************************************/
/**************************************************************************/
// Protected stuff
/**************************************************************************/
/**************************************************************************/

// Main weeds + antz

void FTW_Leaf::resize_weeds() {
  NestInfo* ni = mNest->GetNestInfo();
  int indent_skip = mLevel*ni->GetWIndent();
  int cw = mNest->get_swm_manager()->cell_w();
  int ch = mNest->get_swm_manager()->cell_h();

  if(indent_skip + 4 > ni->GetWName())
    indent_skip = ni->GetWName() - 4;

  wIndentBox->size(indent_skip, 1);
  wExpander->size(2, 1);
  wListExpander->size(2, 1);
  wName->size((ni->GetWName() - indent_skip - 4), 1);
  if(wListDesignation) {
    FTW_Branch* b = (FTW_Branch*)mParent;
    if(b->GetLDShow() && wName->w() > b->GetLDNameMinWidth()) {
      wListDesignation->edit_active(b->GetLDEdit());
      Int_t dw = TMath::Min(b->GetLDWidth(), wName->w() - b->GetLDNameMinWidth());
      wName->size(wName->w() - dw, 1);
      wListDesignation->size(dw, 1);
      wListDesignation->show();
    } else {
      wListDesignation->hide();
    }
  }
  wSepBox->size(ni->GetWSepBox(), 1);
  wFrontPack->size(1,1);

  int ant_w  = ni->GetWAnt();
  int cell_w = mNest->get_swm_manager()->cell_w();
  for(int c=0; c<wAntPack->children(); ++c) {
    FTW_Ant* a = dynamic_cast<FTW_Ant*>(wAntPack->child(c));
    int name_w = ant_w ? ant_w :
      FGS::swm_label_width(a->GetLinkInfo()->fName, cell_w);
    a->resize_weeds(name_w, 2, 1);
  }
  wAntPack->size(1,1);
  wTailBox->size(FTW_Nest::max_W*cw, ch);

  mNest->get_swm_manager()->prepare_group(wFrontPack);
  mNest->get_swm_manager()->prepare_group(wAntPack);
  size(1, ch);
  init_sizes();
}

void FTW_Leaf::label_namebox() {
  if(fImg) {
    wName->set_label(fImg->fLens->GetName());
  } else {
    wName->set_label("<null>");
    wName->labelcolor(FL_DARK_RED);
  }
}

void FTW_Leaf::label_weeds() {
  // just labels the ant part ... virtuality should take care of branches
  if(FractionOfExpandedAnts() <= 0.5) {
    wExpander->label("@#-2->|");
  } else {
    wExpander->label("@#-2->");
  }
  if(wAntPack->children() == 0)
    wExpander->labelcolor(FL_DARK_RED);
  else
    wExpander->labelcolor(FL_BLACK);
  wExpander->redraw();
}

void FTW_Leaf::label_designation()
{
  if(wListDesignation != 0) {
    AList* l = mParent->fImg->GetList();
    if(l->el_type() == AList::ET_Id)
      wListDesignation->value(GForm("%d", mListId));
    else
      wListDesignation->value(mListLabel);
  }
}

void FTW_Leaf::recolor_name() {
  wName->color(mNest->LeafName_Color(this)); wName->redraw();
}

void FTW_Leaf::modify_box_color(Fl_Color mod, bool on_p) {
  Fl_Color c = (Fl_Color)(on_p ? wExpander->color() + mod : wExpander->color() - mod);
  wExpander->color(c);	   wExpander->redraw();
  wListExpander->color(c); wListExpander->redraw();
}

/**************************************************************************/

// Custom View

void FTW_Leaf::create_custom_view(MTW_Layout* layout) {
  if(fImg == 0) return;
  int n = layout->CountSubViews(fImg->fLens);
  if(n > 0) {
    FTW_Shell* shell = mNest->GetShell();
    wCustomView = new MTW_ClassView(fImg, shell);
    wCustomView->BuildByLayout(layout);
    shell->prepare_group(wCustomView);
    insert(*wCustomView, wTailBox);
    wCustomView->hide();
  }
}

void FTW_Leaf::show_custom_view() {
  wAntPack->hide();
  if(wCustomView) wCustomView->show();
}

void FTW_Leaf::hide_custom_view() {
  wAntPack->show();
  if(wCustomView) wCustomView->hide();
}

void FTW_Leaf::wipe_custom_view() {
  if(wCustomView) {
    remove(wCustomView);
    delete wCustomView;
    wCustomView = 0;
  }
}

/**************************************************************************/

void FTW_Leaf::reverse_ants() {
  if(wAntPack->children() > 1) {
    FTW_Ant** a = (FTW_Ant**) wAntPack->array();
    FTW_Ant** b = a + (wAntPack->children() - 1);
    FTW_Ant* c;
    while(b > a) {
      c = *a; *a = *b; *b = c;
      ++a; --b;
    }
    wAntPack->init_sizes();
    wAntPack->redraw();
  }
}
