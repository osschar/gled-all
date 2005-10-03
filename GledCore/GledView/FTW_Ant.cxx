// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "FTW_Ant.h"
#include "FTW_Leaf.h"
#include "FTW_Nest.h"

#include <FL/Fl_Button.H>

namespace OS = OptoStructs;

/**************************************************************************/

static void
cb_ant_collexp(Fl_Button* b, FTW_Ant* a) {
  a->CollExp();
}

static void
cb_ant_list_collexp(Fl_Button* b, FTW_Ant* a) {
  a->ListExpander();
}

/**************************************************************************/
// FTW_Ant
/**************************************************************************/

FTW_Ant::FTW_Ant(OS::ZLinkDatum* ld, FTW_Leaf* p) :
  Fl_Group(0,0,1,1),
  OS::ZLinkView(ld),
  mParent(p), mLeaf(0),
  bExpanded(false), bExpandIfCan(false)
{
  wExpander = new Fl_Button(0,0,1,1);
  wExpander->color(FTW::background_color);
  wExpander->labeltype(FL_SYMBOL_LABEL);
  wExpander->callback((Fl_Callback*)cb_ant_collexp, this);

  wListExpander = new Fl_Button(0,0,1,1,"@#>[]");
  wListExpander->color(FTW::background_color);
  wListExpander->labeltype(FL_SYMBOL_LABEL);
  wListExpander->callback((Fl_Callback*)cb_ant_list_collexp, this);

  wName = new FTW::NameButton(this, 0,0,1,1);
  wName->color(FTW::background_color);
  wName->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
  wName->label(GetLinkInfo()->fName.Data());

  label_weeds();
  resizable(0);
  end();
}

FTW_Ant::~FTW_Ant() {
  delete mLeaf;
}

/**************************************************************************/

void FTW_Ant::Update()
{
  delete mLeaf; mLeaf = 0;
  bool was_expanded = bExpanded;
  bExpanded = false;
  ZLinkView::Update();
  label_weeds();
  if(fToGlass && (bExpandIfCan || was_expanded)) CollExp();
}

/**************************************************************************/

void FTW_Ant::CollExp() {
  if(bExpanded) {
    mParent->CollapseLink(this);
  } else {
    if(fToGlass) mParent->ExpandLink(this);
  }
}

void FTW_Ant::ListExpander() {
  if(bExpanded) {
    mLeaf->CollapseList();
    mParent->CollapseLink(this);
  } else {
    if(fToGlass) mParent->ExpandLink(this);
    if(mLeaf) mLeaf->ExpandList();
  }
}

/**************************************************************************/
/**************************************************************************/

void FTW_Ant::resize_weeds(int name_w, int butt_w, int h) {
  wExpander->resize(0, 0, butt_w, h);
  wListExpander->resize(butt_w, 0, butt_w, h);
  wName->resize(2*butt_w, 0, name_w, h);
  Fl_Widget::resize(0, 0, 2*butt_w + name_w, h);
}

void FTW_Ant::label_weeds() {
  wExpander->labelcolor(FL_BLACK);
  if(bExpanded) {
    wExpander->label("@#-2->");
  } else {
    wExpander->label("@#-2->|");
    if(!GetToImg()) wExpander->labelcolor(FL_DARK_RED);
  }
  wExpander->redraw();

  if(IsList()) {
    wListExpander->labelcolor(FL_BLACK);
  } else {
    wListExpander->labelcolor(FL_DARK_RED);
  }
  wListExpander->redraw();
}

void FTW_Ant::recolor_name() {
  wName->color(mParent->GetNest()->AntName_Color(this)); wName->redraw();
}

void FTW_Ant::modify_box_color(Fl_Color mod, bool on_p) {
  Fl_Color c = (Fl_Color)(on_p ? wExpander->color() + mod : wExpander->color() - mod);
  wExpander->color(c);	   wExpander->redraw();
  wListExpander->color(c); wListExpander->redraw();
}
