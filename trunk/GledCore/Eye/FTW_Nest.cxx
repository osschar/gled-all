// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include <Glasses/NestInfo.h>

#include <Glasses/ZQueen.h>
#include <Gled/GledNS.h>

#include "Eye.h"
#include "FTW_Nest.h"
#include "FTW_Shell.h"
#include "FTW_Leaf.h"
#include "FTW_Ant.h"
#include "MTW_View.h"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/fl_ask.H>



namespace OS = OptoStructs;
namespace GNS  = GledNS;
namespace GVNS = GledViewNS;

/**************************************************************************/

  const int FTW_Nest::def_W = 64;
  const int FTW_Nest::def_H = 30;

  const int FTW_Nest::min_W = 30;
  const int FTW_Nest::max_W = 240;
  const int FTW_Nest::min_H = 6;
  const int FTW_Nest::max_H = 120;

namespace {

  void action_menu_cb(Fl_Menu_Button* b, void* what) {
    FTW_Nest*   nest  = FTW::grep_parent<FTW_Nest*>(b);
    FTW_Shell* shell  = nest->GetShell();
    FTW::Locator& tgt = *nest->RefTargetLoc();
    try {
      switch(int(what)) {
      case 1: shell->X_SetLink(tgt); break;
      case 2: shell->X_ClearLink(tgt); break;

      case 3: shell->X_Yank(tgt); break;

      case 4: shell->X_Push(tgt); break;
      case 5: shell->X_Unshift(tgt); break;
      case 6: shell->X_Insert(tgt); break;

      case 7: shell->X_Pop(tgt); break;
      case 8: shell->X_Shift(tgt); break;
      case 9: shell->X_Remove(tgt); break;
      }
    } catch(string exc) {
      shell->Message(exc.c_str(), FTW_Shell::MT_err);
    }
  }

  void target_type_change_cb(Fl_Widget* w, FTW_Nest::TargetType_e type) {
    FTW_Nest* n = FTW::grep_parent<FTW_Nest*>(w);
    if(n) n->TargetTypeChange(type);
  }

  // *** Menu callbacks ***

  void new_menu_cb(Fl_Menu_Button* b, void* what) {
    FTW_Nest*   nest = FTW::grep_parent<FTW_Nest*>(b);
    FTW_Shell* shell = nest->GetShell();
    ZQueen* q = shell->GetShellInfo()->GetQueen();

    switch(int(what)) {

    case 1: { // Nest
      FID_t fid = GledNS::FindClassID("NestInfo");
      GNS::ClassInfo*  ci = GNS::FindClassInfo("ZList");
      GNS::MethodInfo* mi = ci->FindMethodInfo("Add", false);
      auto_ptr<ZMIR> mir(q->S_InstantiateWAttach(shell->GetShellInfo()->GetNests(), 0,
			 mi->fClassInfo->fFid.lid, mi->fClassInfo->fFid.cid, mi->fMid,
			 fid.lid, fid.cid,
			 GForm("Nest %d", shell->GetShellInfo()->GetNests()->Size()+1),
			 GForm("shell: %s", shell->GetShellInfo()->GetName())));
      nest->fImg->fEye->Send(*mir);
      break;
    }

    case 2: { // Pupil
      FID_t fid = GledNS::FindClassID("PupilInfo");
      GNS::ClassInfo*  ci = GNS::FindClassInfo("ZList");
      GNS::MethodInfo* mi = ci->FindMethodInfo("Add", false);
      auto_ptr<ZMIR> mir(q->S_InstantiateWAttach(nest->GetNestInfo()->GetPupils(), 0,
			 mi->fClassInfo->fFid.lid, mi->fClassInfo->fFid.cid, mi->fMid,
			 fid.lid, fid.cid,
			 GForm("Pupil %d", nest->GetNestInfo()->GetPupils()->Size()+1),
			 GForm("nest: %s", nest->GetNestInfo()->GetName())));
      nest->fImg->fEye->Send(*mir);
      break;
    }

    } // switch
  }

  void view_menu_cb(Fl_Menu_Button* b, void* what) {
    FTW_Nest*   nest = FTW::grep_parent<FTW_Nest*>(b);
    const Fl_Menu_Item* mi = b->mvalue();

    switch(int(what)) {

    case 1: { // Toggle Link / Custom view
      if(nest->GetLinksShown()) nest->CustomView();
      else	      		nest->LinksView();
      break;
    }

    case 2: { // Open Edit Layout Window
      Fl_Window* w = nest->GetLayout();
      w->hotspot(w);
      w->show();
      break;
    }

    case 3: { // Offer some predefined custom layouts; should be pluggable from outside, history !!
      Fl_Menu_Button mb(Fl::event_x_root(), Fl::event_y_root(), 0,0,0);
      mb.textsize(nest->get_swm_manager()->cell_fontsize());

      mb.add("ZGlass", 0, 0, (void*)
	     "ZGlass(Name[20],Title[20],RefCount[6])");
      mb.add("ZNode", 0, 0, (void*)
	     "ZNode(Pos,Rot)");
      mb.add("Eventor", 0, 0, (void*)
	     "Eventor(Running[4],Performing[4],Start[4],Stop[4])");
      mb.add("Saturn monitor", 0, 0, (void*)
	      "SaturnInfo(LAvg1,LAvg5,LAvg15,Memory,MFree,Swap,SFree,CU_Total[6],CU_User[6])");

      const Fl_Menu_Item* cv = mb.popup();
      if(cv) {
	nest->EnactLayout((char*)(cv->user_data()));
	if(nest->GetLinksShown()) nest->CustomView();
      }
      break;
    }

    case 4: { // Reverse order of ants for all leafs
      nest->ReverseAnts();
      break;
    }

    } // end switch
  }

  void set_menu_cb(Fl_Menu_Button* b, void* what) {
    FTW_Nest*   nest  = FTW::grep_parent<FTW_Nest*>(b);
    FTW_Shell* shell  = nest->GetShell();
    switch(int(what)) {
    case 1: shell->X_SetSource(nest->RefPoint()); break;
    case 2: shell->X_SetSink(nest->RefPoint()); break;
    case 3: shell->X_SetSource(nest->RefMark()); break;
    case 4: shell->X_SetSink(nest->RefMark()); break;

    case 16: nest->ExchangePointAndMark(); break;
    case 17: shell->X_ExchangeSourceAndSink(); break;
    }
  }

}

Fl_Menu_Item FTW_Nest::s_New_Menu[] = {
  { "Nest",  0, (Fl_Callback*) new_menu_cb, (void*)1 },
  { "Pupil", 0, (Fl_Callback*) new_menu_cb, (void*)2 },
  {0}
};

Fl_Menu_Item FTW_Nest::s_View_Menu[] = {
  { "Link / Custom",     FL_CTRL + 'v', (Fl_Callback*) view_menu_cb, (void*)1 },
  { "Edit Custom ...",   FL_CTRL + 'e', (Fl_Callback*) view_menu_cb, (void*)2 },
  { "Predef Custom ...", FL_CTRL + 'c', (Fl_Callback*) view_menu_cb, (void*)3, FL_MENU_DIVIDER },
  { "Reverse Ants",      FL_CTRL + 'r', (Fl_Callback*) view_menu_cb, (void*)4, FL_MENU_TOGGLE }, 
  {0}
};

Fl_Menu_Item FTW_Nest::s_Set_Menu[] = {
  { "Point as Source",	FL_F + 1,            (Fl_Callback*) set_menu_cb, (void*)1 },
  { "Point as Sink",	FL_F + 2,            (Fl_Callback*) set_menu_cb, (void*)2 },
  { "Mark as Source",	FL_SHIFT + FL_F + 1, (Fl_Callback*) set_menu_cb, (void*)3 },
  { "Mark as Sink",	FL_SHIFT + FL_F + 2, (Fl_Callback*) set_menu_cb, (void*)4, FL_MENU_DIVIDER },
  { "Exchange Point and Mark",	'x',            (Fl_Callback*) set_menu_cb, (void*)16 },
  { "Exchange Source and Sink",	FL_SHIFT + 'x', (Fl_Callback*) set_menu_cb, (void*)17 },
  {0}
};

Fl_Menu_Item FTW_Nest::s_Action_Menu[] = {
  { "Set Link",		's', (Fl_Callback*) action_menu_cb, (void*)1 },
  { "Clear Link",	'c', (Fl_Callback*) action_menu_cb, (void*)2 },
  { "Yank",		'y', (Fl_Callback*) action_menu_cb, (void*)3 },
  { "Push",		'p', (Fl_Callback*) action_menu_cb, (void*)4 },
  { "Unshift",		'u', (Fl_Callback*) action_menu_cb, (void*)5 },
  { "Insert",		'i', (Fl_Callback*) action_menu_cb, (void*)6 },
  { "Pop",		'o', (Fl_Callback*) action_menu_cb, (void*)7 },
  { "Shift",		'h', (Fl_Callback*) action_menu_cb, (void*)8 },
  { "Remove",		'e', (Fl_Callback*) action_menu_cb, (void*)9 },
  {0}
};

/**************************************************************************/
/**************************************************************************/
// FTW_Nest
/**************************************************************************/
/**************************************************************************/

void FTW_Nest::_build(int w, int h)
{
  mNestInfo = dynamic_cast<NestInfo*>(fImg->fGlass);
  assert(mNestInfo);
  fImg->CheckInFullView(this);

  OS::ZGlassImg* pupils_img = fImg->fEye->DemanglePtr(mNestInfo->GetPupils());
  pPupilAm = new FTW::PupilAm(this, pupils_img);

  mTargetLoc = &mPoint; mTargetType = TT_Point;

  { // Create helpers
    Fl_Group::current(0);

    pCtrl = new FTW_Nest_Ctrl(this);
    pLayout = new MTW_Layout(this);

    Fl_Group::current(this);
  }

  wMainPack = new Fl_Pack(0, 0, w, h-1); // 1 is for InfoBar
  wMainPack->type(FL_VERTICAL);

  // Menu group
  {
    wMenuPack = new Fl_Pack(0, 0, w, 2);
    wMenuPack->type(FL_HORIZONTAL);


    {
      Fl_Group* groo = new Fl_Group(0,0,2,2);
      Fl_Button* b_swm = new Fl_Button(0, 0, 2, 1, "@#3>>");
      b_swm->labeltype(FL_SYMBOL_LABEL);
      set_swm_hotspot_cb(b_swm);

      Fl_Button* b_ctr = new Fl_Button(0,1,2,1, "@#||");
      b_ctr->labeltype(FL_SYMBOL_LABEL);
      pCtrl->set_show_cb(b_ctr);
      groo->end();
    }

    Fl_Menu_Button* but_new = new Fl_Menu_Button(0,0,6,2, "New");
    but_new->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    but_new->menu(s_New_Menu);

    Fl_Menu_Button* but_view = new Fl_Menu_Button(0,0,6,2, "View");
    but_view->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    but_view->menu(s_View_Menu);

    Fl_Menu_Button* but_set = new Fl_Menu_Button(0,0,6,2, "Set");
    but_set->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    but_set->menu(s_Set_Menu);

    Fl_Menu_Button* but_action = new Fl_Menu_Button(0,0,6,2, "Action");
    but_action->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    but_action->menu(s_Action_Menu);

    wMenuPack->end();
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
    }

    wSepBox = new Fl_Box(FTW::separator_box,0,0,1,2,0);
    wSepBox->color(FTW::separator_color);

    wCustomTitle = new Fl_Box(0,0,max_W,2, "Custom Titles");
    wCustomTitle->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    wCustomTitle->box(FL_EMBOSSED_BOX);

    wMidPack->end();
  } // end MidPack

  // Scroll pack
  {
    mScroll = new Fl_Scroll(0, 0, w, 1);
    mScroll->type(Fl_Scroll::VERTICAL_ALWAYS);
    mScroll->scrollbar.resize(w-2, 0, 2, 1);
    mScroll->hscrollbar.resize(0,0, w-2, 1);

    mPack   = new Fl_Pack(0, 0, 512, 1); // Wide enough ...
    mPack->type(FL_VERTICAL);

    mPack->end();
    mScroll->end();

    swm_size_range = new SWM_Size_Range(min_W, min_H, max_W, max_H);
  }

  wMainPack->end();
  wInfoBar = new Fl_Output(0,h-1,w,1);
  end();

  wMainPack->resizable(mScroll);
  resizable(wMainPack);

  label_nest();
}

/**************************************************************************/

FTW_Nest::FTW_Nest(FTW_Shell* sh, OptoStructs::ZGlassImg* img, int w, int h) :
  OS::A_View(img), Fl_Window(w, h), mW(w),
  mShell(sh),
  mPoint(this), mMark(this), mBelowMouse(this)
{
  _build(w, h);
}

FTW_Nest::FTW_Nest(FTW_Shell* sh, OptoStructs::ZGlassImg* img, int x, int y, int w, int h) :
  OS::A_View(img), Fl_Window(x, y, w, h), mW(w),
  mShell(sh),
  mPoint(this), mMark(this), mBelowMouse(this)
{
  _build(w, h);
}

FTW_Nest::~FTW_Nest() {
  // !!!! here should clear all locator consumers with destroy_base()!!!!
  delete pPupilAm;
  fImg->CheckOutFullView(this);
  delete pCtrl;
  delete pLayout;
}

/**************************************************************************/

void FTW_Nest::Absorb_Change(LID_t lid, CID_t cid)
{
  if((lid==1 && cid==1) || (lid==0 && cid==0)) {
    label_nest();
  }
}

/**************************************************************************/

void FTW_Nest::Rewidth()
{
  for(int c=0; c<mPack->children(); ++c) {
    dynamic_cast<FTW_Leaf*>(mPack->child(c))->resize_weeds();
  }
  mPack->redraw();

  // And fix CustomWeed offset
  int cw = swm_manager->cell_w();
  int ch = swm_manager->cell_h();

  int gw1 = (pCtrl->GetWName() - 7) / 3;
  int gw2 = pCtrl->GetWName() - 7 - 2*gw1;
  wTargetPack->child(2)->size(gw1*cw, 2*ch);
  wTargetPack->child(3)->size(gw1*cw, 2*ch);
  wTargetPack->child(4)->size(gw2*cw, 2*ch);

  wSepBox->size(pCtrl->GetWSepBox()*cw, 2*ch);

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

void FTW_Nest::SetSWM(Fl_SWM_Manager* swm, bool self_p)
{
  if(self_p) swm->adopt_window(this);
  swm->adopt_window(pCtrl);
  swm->adopt_window(pLayout);
  Rewidth();
}

/**************************************************************************/

void FTW_Nest::InsertLeaf(FTW_Leaf* newleaf, int at) {
  mPack->insert(*newleaf, at);
  AdInsertio(newleaf);
  mScroll->redraw();
}

void FTW_Nest::InsertLeaf(FTW_Leaf* newleaf, FTW_Leaf* before) {
  mPack->insert(*newleaf, before);
  AdInsertio(newleaf);
  mScroll->redraw();
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
  mScroll->redraw();
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
  while(--pos >= 0) {
    if(dynamic_cast<FTW_Leaf*>(mPack->child(pos))->mLevel <= leaf->mLevel)
       return pos;
  }
  return pos;
}

FTW_Leaf* FTW_Nest::LeafAt(int pos)
{
  if(pos < 0 || pos >= mPack->children()) return 0;
  return dynamic_cast<FTW_Leaf*>(mPack->child(pos));
}

/**************************************************************************/

FTW_Leaf* FTW_Nest::VisibleLeafBefore(FTW_Leaf* leaf)
{
  int c = mPack->find(leaf);
  if(c == mPack->children()) return 0;
  while(--c >= 0) {
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
  return dynamic_cast<FTW_Leaf*>(mPack->child(0));
}

FTW_Leaf* FTW_Nest::LastVisibleLeaf()
{
  int c = mPack->children();
  while(--c >= 0) {
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
  if(layout) { pLayout->GetLaySpecs()->value(layout); pLayout->GetLaySpecs()->redraw(); }
  try {
    pLayout->Parse();
  }
  catch(string exc) {
    cout <<"FTW_Contents_Nest::EnactLayout parse failed: " << exc <<endl;
    return;
  }

  if(bCustomWeedsCreated) {
    wipe_custom_weeds();
  }
  if(!bLinksShown) {
    create_custom_weeds();
    show_custom_weeds();
    mScroll->redraw();
  }
}

void FTW_Nest::ShowRnrCtrls()
{

}

void FTW_Nest::HideRnrCtrls()
{

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
  for(int c=0; c<mPack->children(); ++c) {
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

int FTW_Nest::handle(int ev)
{
  if(ev == FL_SHORTCUT && Fl::event_key() == FL_Escape && parent() == 0) {
    iconize();
    return 1;
  }
  if(ev == FL_ENTER || ev == FL_PUSH) { Fl::focus(this); }
  if(ev == FL_FOCUS) return 1;

  int ret = 0;
  if(ev == FL_KEYBOARD) {
    ret = 1;
    FTW::Locator& loc = Fl::event_state(FL_SHIFT) ? mMark : mPoint;

    try {

      switch(Fl::event_key()) {

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

	// case 'x': ExchangePointAndMark(); break;

	/*
	  case 's': mShell->X_SetLink(*mTargetLoc); break;
	  case 'c': mShell->X_ClearLink(*mTargetLoc); break;

	  case 'y': mShell->X_Yank(*mTargetLoc); break;

	  case 'p': mShell->X_Push(*mTargetLoc); break;
	  case 'u': mShell->X_Unshift(*mTargetLoc); break;
	  case 'i': mShell->X_Insert(*mTargetLoc); break;

	  case 'o': mShell->X_Pop(*mTargetLoc); break;
	  case 'h': mShell->X_Shift(*mTargetLoc); break;
	  case 'e': mShell->X_Remove(*mTargetLoc); break;
	*/

      default: ret = 0;

      }// end switch

    } catch(string exc) {
      mShell->Message(exc.c_str(), FTW_Shell::MT_err);
      fl_beep(FL_BEEP_ERROR);
      return 1;
    }

  }

  if(ret == 0) ret = Fl_Window::handle(ev);

  return ret;
}

/**************************************************************************/
// protected: custom weeds handlers
/**************************************************************************/

void FTW_Nest::create_custom_weeds()
{
  for(int c=0; c<mPack->children(); ++c) {
    FTW_Leaf* l = dynamic_cast<FTW_Leaf*>(mPack->child(c));
    l->create_custom_view(pLayout);
  }
  bCustomWeedsCreated = true;
  // create new child ... or better ... let pLayout produce a Fl_Group
  // hide it
  wCustomLabels = pLayout->CreateLabelGroup();
  swm_manager->prepare_group(wCustomLabels);
  wMidPack->add(wCustomLabels);
  wCustomLabels->hide();
}

void FTW_Nest::show_custom_weeds()
{
  for(int c=0; c<mPack->children(); ++c) {
    FTW_Leaf* l = dynamic_cast<FTW_Leaf*>(mPack->child(c));
    l->show_custom_view();
  }
  wCustomTitle->hide();
  wCustomLabels->show();
  wMidPack->redraw();
}

void FTW_Nest::hide_custom_weeds()
{
  for(int c=0; c<mPack->children(); ++c) {
    FTW_Leaf* l = dynamic_cast<FTW_Leaf*>(mPack->child(c));
    l->hide_custom_view();
  }
  mScroll->redraw();
  wCustomLabels->hide();
  wCustomTitle->show();
  wMidPack->redraw();
}

void FTW_Nest::wipe_custom_weeds()
{
  for(int c=0; c<mPack->children(); ++c) {
    FTW_Leaf* l = dynamic_cast<FTW_Leaf*>(mPack->child(c));
    l->wipe_custom_view();
  }
  bCustomWeedsCreated = false;
  wMidPack->remove(wCustomLabels);
  delete wCustomLabels; wCustomLabels = 0;
}

/**************************************************************************/

void FTW_Nest::label_nest()
{
  label(GForm("nest: %s; shell: %s", mNestInfo->GetName(),
 	                             mShell->GetShellInfo()->GetName()));
  redraw();
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
// FTW_Nest_Ctrl
/**************************************************************************/

namespace {

  char* var_names[] = { "Name", "Ant", "Indent", "SepBox" };
  int var_defs[] = { 30, 12, 2, 1 };

  void ftwnc_setdef_cb(Fl_Button* b, FTW_Nest_Ctrl* n) {
    for(int i=0; i<4; ++i) {
      var_defs[i] = int(n->weeds[i]->value());
    }
  }

  void ftwnc_hide_cb(Fl_Button* b, FTW_Nest_Ctrl* n) { n->hide(); }

  void ftwnc_show_cb(Fl_Button* b, FTW_Nest_Ctrl* n) {
    n->hotspot(n); n->show();
  }

  void ftwnc_change_cb(Fl_Button* b, FTW_Nest* nest) { nest->Rewidth(); }

}

/**************************************************************************/

FTW_Nest_Ctrl::FTW_Nest_Ctrl(FTW_Nest* nest) :
  Fl_Window(24, 3, "FTW_Nest_Ctrl")
{
  for(int i=0; i<4; ++i) {
    weeds[i] = new Fl_Value_Input(i*6, 1, 6, 1, var_names[i]);
    weeds[i]->value(var_defs[i]);
    weeds[i]->align(FL_ALIGN_TOP);
    weeds[i]->callback((Fl_Callback*)ftwnc_change_cb, nest);
  }
  weeds[0]->range(18,50); weeds[0]->step(1);
  weeds[1]->range( 6,40); weeds[1]->step(1);
  weeds[2]->range( 0,10); weeds[2]->step(1);
  weeds[3]->range( 0,5);  weeds[3]->step(1);

  Fl_Button* b;
  b = new Fl_Button(0,2,12,1, "Set Default");
  b->callback((Fl_Callback*) ftwnc_setdef_cb, this);
  b = new Fl_Button(12,2,12,1, "Hide");
  b->callback((Fl_Callback*) ftwnc_hide_cb, this);

  end();
}

void FTW_Nest_Ctrl::set_show_cb(Fl_Widget* w)
{
  w->callback((Fl_Callback*) ftwnc_show_cb, this);
}

void FTW_Nest_Ctrl::set_vals(int* v)
{
  for(int i=0; i<4; ++i) weeds[i]->value(v[i]);
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
