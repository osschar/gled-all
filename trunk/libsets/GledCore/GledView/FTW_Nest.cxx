// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include <Glasses/NestInfo.h>
#include <Glasses/ShellInfo.h>

#include <Glasses/ZQueen.h>
#include <Gled/GledNS.h>
#include <Stones/ZMIR.h>
#include <Eye/Eye.h>

#include "FTW_Shell.h"
#include "FTW_Window.h"
#include "FTW_Nest.h"
#include "FTW_Leaf.h"
#include "FTW_Ant.h"
#include "MTW_View.h"
#include "FltkGledStuff.h"

#include <FL/Fl.H>
#include <FL/Fl_ScrollPack.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/fl_ask.H>

namespace OS   = OptoStructs;
namespace GNS  = GledNS;
namespace GVNS = GledViewNS;
namespace FGS  = FltkGledStuff;

/**************************************************************************/
// creator foo
/**************************************************************************/

FTW_Nest* FTW_Nest::Create_FTW_Nest(FTW_Shell* sh, OS::ZGlassImg* img)
{
  static const Exc_t _eh("FTW_Nest::Create_FTW_Nest ");

  NestInfo* ni = dynamic_cast<NestInfo*>(img->fLens);
  if(ni == 0) throw(_eh + "user-data is not NestInfo.");

  FTW_Window *win  = new FTW_Window(ni->GetDefW(), ni->GetDefH());
  FTW_Nest   *nest = new FTW_Nest(sh, win, img, ni->GetDefW(), ni->GetDefH());
  win->end();
  win->resizable(win);
  win->set_size_range(new SWM_Size_Range(min_W, min_H, max_W, max_H));

  sh->adopt_window(win);
  sh->adopt_window(nest->pLayout);
  nest->Rewidth();

  FTW_Leaf* top_leaf = FTW_Leaf::Construct(nest, 0, img, false, false);
  nest->InsertLeaf(top_leaf);
  if(ni->GetShowSelf() == false) {
    top_leaf->SetLevel(-1);
    top_leaf->hide();
  }
  top_leaf->ExpandList();
  if(ni->GetMaxChildExp() > 0) {
    list<FTW_Leaf*> leaves; top_leaf->CopyListLeaves(leaves);
    if(leaves.size() <= ni->GetMaxChildExp()) {
      for(list<FTW_Leaf*>::iterator l=leaves.begin(); l!=leaves.end(); ++l)
	(*l)->ExpandList();
    }
  }

  nest->_finalize_build();
  return nest;
}

void *SubShellCreator_GledCore_FTW_Nest = (void*)FTW_Nest::Create_FTW_Nest;

/**************************************************************************/

const int FTW_Nest::def_W = 64;
const int FTW_Nest::def_H = 30;

const int FTW_Nest::min_W = 30;
const int FTW_Nest::max_W = 240;
const int FTW_Nest::min_H = 6;
const int FTW_Nest::max_H = 120;

/**************************************************************************/

namespace
{
  void action_menu_cb(Fl_Menu_Button* b, void* what)
  {
    FTW_Nest*   nest  = FGS::grep_parent<FTW_Nest*>(b);
    FTW_Shell* shell  = nest->GetShell();
    FTW::Locator& tgt = *nest->RefTargetLoc();
    try
    {
      switch(GNS::CastVoidPtr2ID(what))
      {
	case 1: shell->X_SetLinkOrElement(tgt); break;
	case 2: shell->X_ClearLinkOrElement(tgt); break;

	case 3: shell->X_Yank(tgt); break;

	case 4: shell->X_Push(tgt); break;
	case 5: shell->X_Unshift(tgt); break;
	case 6: shell->X_Insert(tgt); break;

	case 7: shell->X_Pop(tgt); break;
	case 8: shell->X_Shift(tgt); break;
	case 9: shell->X_Remove(tgt); break;
      }
    }
    catch (Exc_t& exc)
    {
      shell->Message(exc.Data(), ISerror);
    }
  }

  void target_type_change_cb(Fl_Widget* w, FTW_Nest::TargetType_e type)
  {
    FTW_Nest* n = FGS::grep_parent<FTW_Nest*>(w);
    if(n) n->TargetTypeChange(type);
  }

  // *** Menu callbacks ***

  void view_menu_cb(Fl_Menu_Button* b, void* what)
  {
    FTW_Nest*   nest = FGS::grep_parent<FTW_Nest*>(b);
    // const Fl_Menu_Item* mi = b->mvalue();

    switch(GNS::CastVoidPtr2ID(what))
    {
      case 1: // Toggle Link / Custom view
      {
	if(nest->GetLinksShown()) nest->CustomView();
	else	      		nest->LinksView();
	break;
      }
      case 2: // Open Edit Layout Window
      {
	Fl_Window* w = nest->GetLayout();
	w->hotspot(w);
	w->show();
	break;
      }

      // case 3: not used

      case 4: // Reverse order of ants for all leafs.
      {
	nest->ReverseAnts();
	break;
      }
      case 5: // Remove stand-alone MTV_Views.
      {
	nest->GetShell()->RemoveMTW_Views();
	break;
      }
      case 6: // Import all kings as top-levels.
      {
	auto_ptr<ZMIR> mir( nest->GetNestInfo()->S_ImportKings() );
	nest->GetShell()->Send(*mir);
	break;
      }

    } // end switch
  }

  void set_menu_cb(Fl_Menu_Button* b, void* what)
  {
    FTW_Nest*   nest  = FGS::grep_parent<FTW_Nest*>(b);
    FTW_Shell* shell  = nest->GetShell();
    switch (GNS::CastVoidPtr2ID(what))
    {
      case 1: shell->X_SetSource(nest->RefPoint()); break;
      case 2: shell->X_SetSink(nest->RefPoint()); break;
      case 3: shell->X_SetSource(nest->RefMark()); break;
      case 4: shell->X_SetSink(nest->RefMark()); break;

      case 16: nest->ExchangePointAndMark(); break;
      case 17: shell->X_ExchangeSourceAndSink(); break;
    }
  }

  Fl_Menu_Item s_View_Menu[] =
  {
    { "Link / Custom",     FL_CTRL + 'v', (Fl_Callback*) view_menu_cb, (void*)1 },
    { "Edit Custom ...",   FL_CTRL + 'e', (Fl_Callback*) view_menu_cb, (void*)2 },
    { "Reverse Ants",      FL_CTRL + 'r', (Fl_Callback*) view_menu_cb, (void*)4, FL_MENU_TOGGLE | FL_MENU_DIVIDER },
    { "Remove Transients",             0, (Fl_Callback*) view_menu_cb, (void*)5, FL_MENU_DIVIDER },
    { "Import Kings",                  0, (Fl_Callback*) view_menu_cb, (void*)6 },
    {0}
  };

  Fl_Menu_Item s_Set_Menu[] =
  {
    { "Point as Source",                FL_F + 1, (Fl_Callback*) set_menu_cb, (void*)1 },
    { "Point as Sink",                  FL_F + 2, (Fl_Callback*) set_menu_cb, (void*)2 },
    { "Mark as Source",      FL_SHIFT + FL_F + 1, (Fl_Callback*) set_menu_cb, (void*)3 },
    { "Mark as Sink",        FL_SHIFT + FL_F + 2, (Fl_Callback*) set_menu_cb, (void*)4, FL_MENU_DIVIDER },
    { "Exchange Point and Mark",             'x', (Fl_Callback*) set_menu_cb, (void*)16 },
    { "Exchange Source and Sink", FL_SHIFT + 'x', (Fl_Callback*) set_menu_cb, (void*)17 },
    {0}
  };

  Fl_Menu_Item s_Action_Menu[] =
  {
    { "Set Link",	's', (Fl_Callback*) action_menu_cb, (void*)1 },
    { "Clear Link",	'c', (Fl_Callback*) action_menu_cb, (void*)2 },
    { "Yank",		'y', (Fl_Callback*) action_menu_cb, (void*)3 },
    { "Push",		'p', (Fl_Callback*) action_menu_cb, (void*)4 },
    { "Unshift",	'u', (Fl_Callback*) action_menu_cb, (void*)5 },
    { "Insert",		'i', (Fl_Callback*) action_menu_cb, (void*)6 },
    { "Pop",		'o', (Fl_Callback*) action_menu_cb, (void*)7 },
    { "Shift",		'h', (Fl_Callback*) action_menu_cb, (void*)8 },
    { "Remove",		'e', (Fl_Callback*) action_menu_cb, (void*)9 },
    {0}
  };

}

/**************************************************************************/
/**************************************************************************/
// FTW_Nest
/**************************************************************************/
/**************************************************************************/

void FTW_Nest::_build(int w, int h)
{
  mNestInfo = dynamic_cast<NestInfo*>(fImg->fLens);
  assert(mNestInfo);

  mTargetLoc = &mPoint; mTargetType = TT_Point;

  { // Create helpers
    pLayout = new MTW_Layout(this);
  }

  begin();
  wMainPack = new Fl_Pack(0, 0, w, h);
  wMainPack->type(FL_VERTICAL);

  // Menu group
  {
    wMenuPack = new Fl_Pack(0, 0, w, 2);
    wMenuPack->type(FL_HORIZONTAL);

    new FGS::MenuBox(s_View_Menu,   4, 2, "View");

    {
      FGS::LensChoiceMenuBox* view_sel = new FGS::LensChoiceMenuBox
	(fImg, 0, 0, 6, 2, "Layouts");
      view_sel->SetSrcImg(fImg);
      view_sel->SetSrcLinkName("LayoutList");
      view_sel->SetSrcFid(FID_t(0,0)); // anything goes
      view_sel->SetSrcConfigPath(NestInfo::sLayoutPath);
      view_sel->SetMethodInfo(fImg->GetCI()->
			      FindMethodInfo("ImportLayout", true));
      view_sel->box((Fl_Boxtype)GVNS::menubar_box);
    }

    new FGS::MenuBox(s_Set_Menu,    4, 2, "Set");
    new FGS::MenuBox(s_Action_Menu, 5, 2, "Action");

    FGS::LensRepNameBox* self = new FGS::LensRepNameBox(fImg, 0, 0, max_W, 2);
    self->box((Fl_Boxtype)GVNS::menubar_box);
    self->color(fl_rgb_color(220,200,200));

    wMenuPack->end();
    wMenuPack->resizable(0);
  }

  // Mid-pack: view-ctrl & custom weed titles
  wCustomLabels = 0;
  bLinksShown = true;
  bCustomWeedsCreated = false;
  bAntsReversed = false;
  {
    wMidPack = new Fl_Pack(0,2,1,2); wMidPack->type(FL_HORIZONTAL);

    {
      wTargetPack = new Fl_Pack(0,0,1,2);
      wTargetPack->type(FL_HORIZONTAL);

      Fl_Box* b = new Fl_Box(FL_FLAT_BOX, 0,0,6,2, "Target");
      b->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
      b->labelsize( labelsize() + 1 );
      b->labelfont(FL_HELVETICA_BOLD);
      b->color(FTW::background_color + FTW::target_modcol);

      b = new Fl_Box(FTW::separator_box, 0,0,1,2,0); b->color(FTW::separator_color);

      Fl_Button* but;
      but = new Fl_Light_Button(0,0,6,2,"Point");
      but->color(FTW::background_color + FTW::target_modcol);
      but->type(FL_RADIO_BUTTON); but->selection_color(FL_RED);
      but->callback((Fl_Callback*) target_type_change_cb, (void*)TT_Point);
      but->value(1);
      but = new Fl_Light_Button(0,0,6,2,"Mark");
      but->type(FL_RADIO_BUTTON); but->selection_color(FL_RED);
      but->callback((Fl_Callback*) target_type_change_cb, (void*)TT_Mark);
      but = new Fl_Light_Button(0,0,6,2,"Below Mouse");
      but->type(FL_RADIO_BUTTON); but->selection_color(FL_RED);
      but->callback((Fl_Callback*) target_type_change_cb, (void*)TT_BelowMouse);
      but->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);

      wTargetPack->end();
      wTargetPack->resizable(0);
    }

    wSepBox = new Fl_Box(FTW::separator_box,0,0,1,2,0);
    wSepBox->color(FTW::separator_color);

    wCustomTitle = new Fl_Box(0,0,max_W,2, "All Links");
    wCustomTitle->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    wCustomTitle->box(FL_EMBOSSED_BOX);

    wMidPack->end();
    wMidPack->resizable(0);
  } // end MidPack

  // Scrollpack
  {
    mPack = new Fl_ScrollPack(0, 0, w, h-5);
    mPack->end();
  }

  wInfoBar = new Fl_Output(0,h-1,w,1);
  wMainPack->end();
  end();

  wMainPack->resizable(mPack);
  resizable(wMainPack);

  label_window();
}

void FTW_Nest::_finalize_build()
{
  if(mNestInfo->GetLeafLayout() == NestInfo::LL_Custom) {
    EnactLayout(mNestInfo->GetLayout());
    CustomView();
  }
}

/**************************************************************************/

FTW_Nest::FTW_Nest(FTW_Shell* sh, Fl_Window* win, OptoStructs::ZGlassImg* img, int w, int h) :
  FTW_SubShell(sh, win, this),
  OS::A_View(img),
  Fl_Group(0, 0, w, h),
  mW(w),
  mPoint(this), mMark(this), mBelowMouse(this)
{
  end();
  _build(w, h);
}

FTW_Nest::~FTW_Nest() {
  // !!!! here should clear all locator consumers with destroy_base()!!!!
  delete pLayout;
}

/**************************************************************************/

void FTW_Nest::AbsorbRay(Ray& ray)
{
  if(ray.IsBasicChange()) {
    label_window();
    return;
  }

  if(ray.fFID == NestInfo::FID()) {
    switch (ray.fRQN)
      {
      case NestInfo::PRQN_layout_change: {
	switch (mNestInfo->GetLeafLayout()) {
	case NestInfo::LL_Ants:
	  LinksView();
	  break;
	case NestInfo::LL_Custom:
	  EnactLayout(mNestInfo->GetLayout());
	  CustomView();
	  break;
	}
	break;
      }
      case NestInfo::PRQN_rewidth:
	Rewidth();
	break;
      }
    return;
  }
}

/**************************************************************************/

void FTW_Nest::Rewidth()
{
  for(int c=1; c<mPack->children(); ++c) {
    dynamic_cast<FTW_Leaf*>(mPack->child(c))->resize_weeds();
  }
  mPack->redraw();

  // And fix CustomWeed offset
  int cw = mShell->cell_w();
  int ch = mShell->cell_h();

  int gw1 = (mNestInfo->GetWName() - 7) / 3;
  int gw2 = mNestInfo->GetWName() - 7 - 2*gw1;
  wTargetPack->child(2)->size(gw1*cw, 2*ch);
  wTargetPack->child(3)->size(gw1*cw, 2*ch);
  wTargetPack->child(4)->size(gw2*cw, 2*ch);

  wSepBox->size(mNestInfo->GetWSepBox()*cw, 2*ch);

  wMidPack->redraw();
}

void FTW_Nest::AdInsertio(FTW_Leaf* newleaf)
{
  if(bCustomWeedsCreated) {
    newleaf->create_custom_view(pLayout);
  }
  if(!bLinksShown) {
    newleaf->show_custom_view();
  }
}

/**************************************************************************/

void FTW_Nest::InsertLeaf(FTW_Leaf* newleaf, int at) {
  mPack->insert(*newleaf, at);
  AdInsertio(newleaf);
  mPack->redraw();
}

void FTW_Nest::InsertLeaf(FTW_Leaf* newleaf, FTW_Leaf* before) {
  mPack->insert(*newleaf, before);
  AdInsertio(newleaf);
  mPack->redraw();
}

void FTW_Nest::RemoveLeaf(FTW_Leaf* exleaf) {
  // !!!! Check Point/Mark; if so, notify shell for Src/Sink check
  if(exleaf == mPoint.leaf) MigrateLocator(mPoint);
  if(exleaf == mMark.leaf)  MigrateLocator(mMark);
  if(exleaf == mBelowMouse.leaf) mBelowMouse.clear();

  for(mpLoc2pLCon_i i=mConsumerMap.begin(); i!=mConsumerMap.end(); ++i) {
    if(i->first->leaf == exleaf) {
      // This is (maybe) not the best thing to do .... at least sometimes
      MigrateLocator(*i->first);
      /*
      for(list<FTW::LocatorConsumer*>::iterator j=i->second.begin(); j!=i->second.end(); ++j) {
	MigrateLocator(*j);
      }
      */
    }
  }

  exleaf->hide();
  mPack->remove(*exleaf);
  mPack->redraw();
}

/**************************************************************************/

int FTW_Nest::PackPos(FTW_Leaf* leaf) {
  return mPack->find(leaf);
}

int FTW_Nest::PackPosAfter(FTW_Leaf* leaf) {
  int pos = mPack->find(leaf);
  while(++pos < mPack->children()) {
    if(dynamic_cast<FTW_Leaf*>(mPack->child(pos))->mLevel <= leaf->mLevel)
       return pos;
  }
  return pos;
}

int FTW_Nest::PackPosBefore(FTW_Leaf* leaf) {
  int pos = mPack->find(leaf);
  while(--pos > 0) {
    if(dynamic_cast<FTW_Leaf*>(mPack->child(pos))->mLevel <= leaf->mLevel)
       return pos;
  }
  return pos;
}

FTW_Leaf* FTW_Nest::LeafAt(int pos)
{
  if(pos < 1 || pos >= mPack->children()) return 0;
  return dynamic_cast<FTW_Leaf*>(mPack->child(pos));
}

/**************************************************************************/

FTW_Leaf* FTW_Nest::VisibleLeafBefore(FTW_Leaf* leaf)
{
  int c = mPack->find(leaf);
  if(c == mPack->children()) return 0;
  while(--c > 0) { // child 0 is scrollbar
    FTW_Leaf* l = dynamic_cast<FTW_Leaf*>(mPack->child(c));
    if(l->visible()) return l;
  }
  return 0;
}

FTW_Leaf* FTW_Nest::VisibleLeafAfter(FTW_Leaf* leaf)
{
  int c = mPack->find(leaf);
  while(++c < mPack->children()) {
    FTW_Leaf* l = dynamic_cast<FTW_Leaf*>(mPack->child(c));
    if(l->visible()) return l;
  }
  return 0;
}

FTW_Leaf* FTW_Nest::FirstVisibleLeaf()
{
  return dynamic_cast<FTW_Leaf*>(mPack->child(1));
}

FTW_Leaf* FTW_Nest::LastVisibleLeaf()
{
  int c = mPack->children();
  while(--c > 0) {
    FTW_Leaf* l = dynamic_cast<FTW_Leaf*>(mPack->child(c));
    if(l->visible()) return l;
  }
  return 0;
}

/**************************************************************************/
// View modes
/**************************************************************************/

void FTW_Nest::LinksView()
{
  if(!bLinksShown) {
    hide_custom_weeds();
    bLinksShown = true;
  }
}

void FTW_Nest::CustomView()
{
  if(pLayout->GetIsValid() && bLinksShown) {
    if(!bCustomWeedsCreated) {
      create_custom_weeds();
    }
    show_custom_weeds();
    bLinksShown = false;
  }
}

void FTW_Nest::EnactLayout(const char* layout)
{
  static const Exc_t _eh("FTW_Nest::EnactLayout ");

  if(layout) { pLayout->GetLaySpecs()->value(layout); pLayout->GetLaySpecs()->redraw(); }
  try {
    pLayout->Parse(mShell->cell_w());
  }
  catch(Exc_t& exc) {
    mShell->Message(_eh + "parse failed: '" + exc + "'.", ISerror);
    return;
  }

  if(bCustomWeedsCreated) {
    wipe_custom_weeds();
  }
  if(!bLinksShown) {
    create_custom_weeds();
    show_custom_weeds();
    mPack->redraw();
  }
}

/**************************************************************************/
// Locators
/**************************************************************************/

void FTW_Nest::MigrateLocator(FTW::Locator& loc)
{
  // Migrates locator to next position, trying the following:
  // next list member, previous list member, previous leaf.

  bool done = false;
  if(loc.is_list_member) {
    FTW_Leaf* l = LeafAt(PackPosAfter(loc.leaf));
    if(l && l->mParent == loc.leaf->mParent) {
      loc.set(l);
      done = true;
    } else {
      l = LeafAt(PackPosBefore(loc.leaf));
      if(l && l->mParent == loc.leaf->mParent) {
	loc.set(l);
	done = true;
      }
    }
  }
  if(!done) loc.up();
  loc.clear_old();
}

void FTW_Nest::LocatorChange(FTW::Locator& loc)
{
  mpLoc2pLCon_i i = mConsumerMap.find(&loc);
  if(i != mConsumerMap.end()) {
    for(list<FTW::LocatorConsumer*>::iterator j=i->second.begin(); j!=i->second.end(); ++j) {
      (*j)->locator_change(loc);
    }
  }

  if(&loc == mTargetLoc) {
    loc.mod_old_box_color(FTW::target_modcol, false);
    loc.mod_box_color(FTW::target_modcol, true);
  }

  if(&loc == &mPoint || &loc == &mMark) {
    if(loc.old_leaf) loc.old_leaf->recolor_name();
    if(loc.old_ant) loc.old_ant->recolor_name();
    if(loc.leaf) loc.leaf->recolor_name();
    if(loc.ant) loc.ant->recolor_name();
  }
}

void FTW_Nest::ExchangePointAndMark()
{
  FTW_Leaf* l = mPoint.leaf; FTW_Ant* a = mPoint.ant;
  mPoint.set(mMark.leaf, mMark.ant);
  mMark.set(l, a);
}

void FTW_Nest::TargetTypeChange(TargetType_e tt)
{
  FTW::Locator* ex_loc = mTargetLoc;
  bool err = false;
  switch(tt) {
  case TT_Point:	mTargetLoc = &mPoint; break;
  case TT_Mark:		mTargetLoc = &mMark; break;
  case TT_BelowMouse:	mTargetLoc = &mBelowMouse; break;
  default: 		err = true;
  }
  if(!err) {
    ex_loc->mod_box_color(FTW::target_modcol, false);
    mTargetLoc->mod_box_color(FTW::target_modcol, true);
    wTargetPack->child(mTargetType)->color(FTW::background_color);
    wTargetPack->child(tt)->color(FTW::background_color + FTW::target_modcol);
    mTargetType = tt;
    wTargetPack->redraw();
  }
}

void FTW_Nest::RegisterLocatorConsumer(FTW::LocatorConsumer* lcon)
{
  // No check for multiple entries

  FTW::Locator* loc = lcon->m_base;
  mConsumerMap[loc].push_back(lcon);
}

void FTW_Nest::UnregisterLocatorConsumer(FTW::LocatorConsumer* lcon)
{
  FTW::Locator* loc = lcon->m_base;
  mpLoc2pLCon_i i = mConsumerMap.find(loc);
  if(i != mConsumerMap.end()) {
    list<FTW::LocatorConsumer*>::iterator j =
      std::find(i->second.begin(), i->second.end(), lcon);
    if(j != i->second.end()) {
      i->second.erase(j);
      if(i->second.empty()) {
	if(!IsPoint(*loc) && !IsMark(*loc)) {
	  // cout <<"FTW_Nest::UnregisterLocatorConsumer deleting locator\n";
	  delete loc;
	}
	// cout <<"FTW_Nest::UnregisterLocatorConsumer wiping locator slot in consumers\n";
	mConsumerMap.erase(i);
      }
    } else {
      cout <<"FTW_Nest::UnregisterLocatorConsumer locator consumer not registered\n";
    }
  } else {
    cout <<"FTW_Nest::UnregisterLocatorConsumer locator not registered\n";
  }
}

/**************************************************************************/

void FTW_Nest::SetInfoBar(const char* info)
{
  wInfoBar->value(info);
}

void FTW_Nest::ReverseAnts()
{
  for(int c=1; c<mPack->children(); ++c) {
    FTW_Leaf* l = dynamic_cast<FTW_Leaf*>(mPack->child(c));
    l->reverse_ants();
  }
  bAntsReversed = !bAntsReversed;
}

/**************************************************************************/
// Name / Collapsor coloration
/**************************************************************************/

Fl_Color FTW_Nest::LeafName_Color(FTW_Leaf* leaf)
{
  unsigned int c = FTW::background_color;
  if(leaf == mPoint.leaf) { c += 0x40200000; if(mPoint.ant) c -= 0x10004000; }
  if(leaf == mMark.leaf)  { c += 0x00204000; if(mMark.ant)  c -= 0x40001000; }
  return (Fl_Color)c;
}

Fl_Color FTW_Nest::AntName_Color(FTW_Ant* ant)
{
  unsigned int c = FTW::background_color;
  if(ant == mPoint.ant) c += 0x40200000;
  if(ant == mMark.ant)  c += 0x00204000;
  return (Fl_Color)c;
}

/**************************************************************************/

void FTW_Nest::label_window(const char* l)
{
  if(l == 0) l = GForm("nest: %s '%s'", mNestInfo->GetName(), mNestInfo->GetTitle());
  FTW_SubShell::label_window(l);
}

int FTW_Nest::handle(int ev)
{
  if (ev == FL_SHORTCUT && Fl::event_key() == FL_Escape && parent() == mWindow)
  {
    mWindow->iconize();
    return 1;
  }
  if (ev == FL_ENTER || ev == FL_PUSH) { Fl::focus(this); }
  if (ev == FL_FOCUS) return 1;

  int ret = 0;
  if (ev == FL_KEYBOARD)
  {
    ret = 1;
    FTW::Locator& loc = Fl::event_state(FL_SHIFT) ? mMark : mPoint;

    try
    {
      switch(Fl::event_key())
      {
      case FL_Up:    loc.up(); break;
      case FL_Down:  loc.down(); break;
      case FL_Left:  loc.left(); break;
      case FL_Right: loc.right(); break;

      case FL_Enter:
	if(loc.leaf) {
	  if(loc.ant)	loc.leaf->ExpandLink(loc.ant);
	  else		loc.leaf->ExpandList();
	}
	break;
      case FL_Insert: if(loc.leaf) loc.leaf->CollExp(); break;
      case FL_Delete:
	if(loc.leaf) {
	  if(loc.ant)	loc.leaf->CollapseLink(loc.ant);
	  else		loc.leaf->CollapseList();
	}
	break;

      default: ret = wMenuPack->handle(FL_SHORTCUT);

      }// end switch

    } catch(Exc_t& exc) {
      mShell->Message(exc.Data(), ISerror);
      fl_beep(FL_BEEP_ERROR);
      return 1;
    }

  }
  else if (ev == FL_MOUSEWHEEL)
  {
    static GTime last;
    GTime now(GTime::I_Now);
    double dt = (now - last).ToDouble();
    int f = 0;
    if      (dt < 0.01) f = 16;
    else if (dt < 0.03) f =  8;
    else if (dt < 0.05) f =  4;
    else if (dt < 0.1)  f =  2;
    if (f) mPack->scrollbar.linesize(f * GetShell()->cell_h());
    mPack->scrollbar.handle(ev);
    if (f) mPack->scrollbar.linesize(GetShell()->cell_h());
    last = now;
    return 1;
  }

  if(ret == 0) ret = Fl_Group::handle(ev);

  return ret;
}

void FTW_Nest::resize(int x, int y, int w, int h)
{
  // printf("Nest::resize %d %d %d %d inres=%d p=%p\n",
  //   x,y,w,h,mShell->in_rescale_p(),parent());

  if(mShell->in_rescale_p() || parent() == 0) {
    Fl_Group::resize(x,y,w,h);
    return;
  }

  int dw = w - this->w(), dh = h - this->h();
  wMainPack->resizable(0);

  Fl_Group::resize(x,y,w,h);

  mPack->resize(mPack->x(), mPack->y(), mPack->w()+dw, mPack->h()+dh);
  wMainPack->resizable(mPack);
}

/**************************************************************************/
// protected: custom weeds handlers
/**************************************************************************/

void FTW_Nest::create_custom_weeds()
{
  for(int c=1; c<mPack->children(); ++c) {
    FTW_Leaf* l = dynamic_cast<FTW_Leaf*>(mPack->child(c));
    l->create_custom_view(pLayout);
  }
  bCustomWeedsCreated = true;
  // create new child ... or better ... let pLayout produce a Fl_Group
  // hide it
  wCustomLabels = pLayout->CreateLabelGroup();
  mShell->prepare_group(wCustomLabels);
  wMidPack->add(wCustomLabels);
  wCustomLabels->hide();
}

void FTW_Nest::show_custom_weeds()
{
  for(int c=1; c<mPack->children(); ++c) {
    FTW_Leaf* l = dynamic_cast<FTW_Leaf*>(mPack->child(c));
    l->show_custom_view();
  }
  wCustomTitle->hide();
  wCustomLabels->show();
  wMidPack->redraw();
}

void FTW_Nest::hide_custom_weeds()
{
  for(int c=1; c<mPack->children(); ++c) {
    FTW_Leaf* l = dynamic_cast<FTW_Leaf*>(mPack->child(c));
    l->hide_custom_view();
  }
  mPack->redraw();
  wCustomLabels->hide();
  wCustomTitle->show();
  wMidPack->redraw();
}

void FTW_Nest::wipe_custom_weeds()
{
  for(int c=1; c<mPack->children(); ++c) {
    FTW_Leaf* l = dynamic_cast<FTW_Leaf*>(mPack->child(c));
    l->wipe_custom_view();
  }
  bCustomWeedsCreated = false;
  wMidPack->remove(wCustomLabels);
  delete wCustomLabels; wCustomLabels = 0;
}

/**************************************************************************/
