// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.


#include "FTW.h"
#include "FTW_Shell.h"
#include "FTW_Nest.h"
#include "FTW_Leaf.h"
#include "FTW_Ant.h"
#include "OptoStructs.h"
#include "Eye.h"
#include <GledView/GledViewNS.h>

// MIR creators
#include <Stones/ZMIR.h>
#include <Glasses/ZQueen.h>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/fl_draw.H>

#include <memory>

namespace GVNS = GledViewNS;
namespace OS   = OptoStructs;

Fl_Color FTW::background_color = (Fl_Color)0xbfbfbf00;
Fl_Color FTW::separator_color  = (Fl_Color)0xb2b28000;

Fl_Color FTW::target_modcol = (Fl_Color)0x40000000;
Fl_Color FTW::source_modcol = (Fl_Color)0x00400000;
Fl_Color FTW::sink_modcol   = (Fl_Color)0x00004000;

Fl_Boxtype FTW::separator_box  = FL_BORDER_BOX;

/**************************************************************************/
// Locator
/**************************************************************************/

bool FTW::Locator::has_contents() {
  if(leaf) {
    if(ant) return ant->IsSet();
    return true;
  } else {
    return false;
  }
}

bool FTW::Locator::is_list() {
  if(leaf) {
    if(ant) return ant->IsList();
    return leaf->IsList();
  } else {
    return false;
  }
}

/**************************************************************************/

void FTW::Locator::clear() {
  is_link = false; is_list_member = false;
  leaf=old_leaf=0; ant=old_ant=0;
}

void FTW::Locator::clear_old() { old_leaf=0; old_ant=0; }

void FTW::Locator::set(FTW_Leaf* l, FTW_Ant* a) {
  old_leaf = leaf; leaf = l;
  old_ant  = ant;  ant  = a;
  if(leaf) {
    if(ant) {
      is_link = true;
      is_list_member = false;
    } else {
      is_link = false;
      is_list_member = leaf->IsListMember();
    }
  } else {
    is_link = false; is_list_member = false;
  }
  if(nest) nest->LocatorChange(*this);
}

void FTW::Locator::revert() {
  set(old_leaf, old_ant);
}

/**************************************************************************/

// Call **has_contents()** before calling any of the following!

ID_t FTW::Locator::get_contents() {
  return ant ? ant->fToImg->fGlass->GetSaturnID() : leaf->fImg->fGlass->GetSaturnID();
}

ZGlass* FTW::Locator::get_glass()
{ return ant ? ant->fToImg->fGlass : leaf->fImg->fGlass; }

OS::ZGlassImg* FTW::Locator::get_image()
{ return ant ? ant->fToImg : leaf->fImg; }

//ID_t FTW::Locator::get_id()
//{ return ant ? ant->fToImg->fGlass->GetSaturnID() : leaf->fImg->fGlass->GetSaturnID(); }

GledViewNS::ClassInfo* FTW::Locator::get_class_info()
{ return ant ? ant->fToImg->fClassInfo : leaf->fImg->fClassInfo; }


ZGlass* FTW::Locator::get_leaf_glass()
{ return leaf->fImg->fGlass; }

ID_t FTW::Locator::get_leaf_id()
{ return leaf->fImg->fGlass->GetSaturnID(); }

/**************************************************************************/

void FTW::Locator::up() {
  FTW_Leaf* l = leaf ? nest->VisibleLeafBefore(leaf) : nest->LastVisibleLeaf();
  FTW_Ant*  a = 0;
  if(l && leaf && ant) a = l->AntAt( leaf->AntPos(ant) );
  set(l, a);
}

void FTW::Locator::down() {
  FTW_Leaf* l = leaf ? nest->VisibleLeafAfter(leaf) : nest->FirstVisibleLeaf();
  FTW_Ant*  a = 0;
  if(l && leaf && ant) a = l->AntAt( leaf->AntPos(ant) );
  set(l, a);
}

void FTW::Locator::left() {
  if(leaf==0) return;
  FTW_Ant*  a = 0;
  if(ant) {
    if(leaf->AntPos(ant) > 0)
      a = leaf->AntAt( leaf->AntPos(ant) - 1 );
    else
      a = 0;
  } else {
    a = leaf->LastAnt();
  }
  if(ant!=a) set(leaf, a);
}

void FTW::Locator::right() {
  if(leaf==0) return;
  FTW_Ant*  a = 0;
  if(ant) {
    a = leaf->AntAt( leaf->AntPos(ant) + 1 );
  } else {
    a = leaf->AntAt(0);
  }
  if(ant!=a) set(leaf, a);
}

/**************************************************************************/

void FTW::Locator::mod_old_box_color(Fl_Color mod, bool on_p)
{
  if(old_leaf)
    if(old_ant)
      old_ant->modify_box_color(mod, on_p);
    else
      old_leaf->modify_box_color(mod, on_p);
}

void FTW::Locator::mod_box_color(Fl_Color mod, bool on_p)
{
  if(leaf)
    if(ant)
      ant->modify_box_color(mod, on_p);
    else
      leaf->modify_box_color(mod, on_p);
}

/**************************************************************************/
// LocatorCunsumer
/**************************************************************************/

void FTW::LocatorConsumer::set_base(Locator& loc)
{
  if(m_base) m_base->nest->UnregisterLocatorConsumer(this);
  m_base = &loc;
  if(m_base) m_base->nest->RegisterLocatorConsumer(this);
}

void FTW::LocatorConsumer::clear_base()
{
  if(m_base) m_base->nest->UnregisterLocatorConsumer(this);
  m_base = 0;
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
// NameButton
/**************************************************************************/

FTW::NameButton::NameButton(Loc_e l, int x, int y, int w, int h, const char* t)
  : Fl_Button(x,y,w,h,t), m_loc(l)
{}

FTW::NameButton::NameButton(FTW_Leaf* leaf, int x, int y, int w, int h, const char* t)
  : Fl_Button(x,y,w,h,t), m_loc(L_Leaf)
{}

FTW::NameButton::NameButton(FTW_Ant* ant, int x, int y, int w, int h, const char* t)
  : Fl_Button(x,y,w,h,t), m_loc(L_Ant)
{}

/**************************************************************************/
// protected methods

FTW_Leaf* FTW::NameButton::get_leaf()
{
  switch(m_loc) {
  case L_Leaf: return dynamic_cast<FTW_Leaf*>(parent()->parent());
  case L_Ant:  return dynamic_cast<FTW_Ant*>(parent())->GetParent();
  default:     return 0;
  }
}

FTW_Ant* FTW::NameButton::get_ant()
{
  if(m_loc == L_Ant) {
    return dynamic_cast<FTW_Ant*>(parent());
  } else {
    return 0;
  }
}

/**************************************************************************/
// public ...

int FTW::NameButton::handle(int ev)
{
  //if(m_loc == L_undef) return Fl_MC_Widget<Fl_Button>::handle(ev);

  static int x, y, dx, dy;
  FTW_Leaf* leaf = get_leaf();
  FTW_Ant*  ant  = get_ant();

  switch(ev) {

  case FL_ENTER: {
    if(leaf) {
      leaf->GetNest()->RefBelowMouse().set(leaf, ant);
      GledViewNS::ClassInfo* ci = leaf->fImg->fClassInfo;
      ZGlass* lens = leaf->fImg->fGlass;
      if(ant == 0) {
	leaf->GetNest()->SetInfoBar(GForm("%s (\"%s\",\"%s\")",
					  ci->fClassName.c_str(),
					  lens->GetName(), lens->GetTitle()));
      } else {
	OptoStructs::ZLinkDatum* ld = ant->fLinkDatum;
	const char *glass = "null", *name = "", *title = "";
	if(ant->fToImg) {
	  glass = ant->fToImg->fClassInfo->fClassName.c_str();
	  name  = ant->fToGlass->GetName();
	  title = ant->fToGlass->GetTitle();
	}
	leaf->GetNest()->SetInfoBar(GForm("%s (\"%s\") [%s %s] -> %s (\"%s\",\"%s\")",
					  ci->fClassName.c_str(),
					  lens->GetName(),
					  ld->fLinkInfo->fType.c_str(),
					  ld->fLinkInfo->fName.c_str(),
					  glass, name, title));
      }
    }
    return 1;
  }

  case FL_LEAVE: {
    if(leaf) {
      leaf->GetNest()->RefBelowMouse().set(0, 0);
    }
    return 1;
  }

  case FL_PUSH: {
    switch(Fl::event_button()) {
    case 1: 
      x = Fl::event_x(); y = Fl::event_y(); dx = dy = 0;
      labelcolor(fl_color_cube(2,0,0)); value(1); redraw();
      return 1;

    case 3: {
      Locator loc(leaf->GetNest(), leaf, ant);
      FTW_Shell *shell = leaf->GetNest()->GetShell();
      shell->LocatorMenu(loc, Fl::event_x_root(), Fl::event_y_root());
      return 1;
    }
    } // end switch

    return 1;
  }


  case FL_DRAG: {
    if(Fl::event_inside(this) && Fl::event_state(FL_BUTTON1)) {
      dx = abs(Fl::event_x() - x); y = abs(Fl::event_y() - y);
      if(dx > w()/7) labelcolor(fl_color_cube(0,0,2));
      else	     labelcolor(fl_color_cube(2,0,0));
      value(1);
    } else {
      labelcolor(FL_BLACK);
      value(0);
    }
    redraw();
    return 1;
  }

  case FL_RELEASE: {
    if(Fl::event_button() == 1 && Fl::event_inside(this)) {
      if(Fl::event_clicks() == 1) {
	FTW_Shell *shell = leaf->GetNest()->GetShell();
	if(m_loc == L_Leaf) {
	  shell->SpawnMTW_View(leaf->fImg);
	  leaf->GetNest()->RefPoint().revert();
	  Fl::event_clicks(0);
	} else if(m_loc == L_Ant && ant->fToImg != 0) {
	  shell->SpawnMTW_View(ant->fToImg);
	  leaf->GetNest()->RefPoint().revert();
	  Fl::event_clicks(0);
	}
      } else {
	if(dx > w()/7) leaf->GetNest()->RefMark().set(leaf, ant);
	else	       leaf->GetNest()->RefPoint().set(leaf, ant);
      }
    }
    labelcolor(FL_BLACK);
    value(0);
    redraw();
    return 1;
  }

  } // switch ev

  return 0;
}

/**************************************************************************/
// NameBox
/**************************************************************************/

FTW::NameBox::NameBox(OS::ZGlassImg* i, int x, int y, int w, int h) :
  A_View(i), Fl_Box(x,y,w,h)
{
  align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
  box(FL_EMBOSSED_BOX);
  if(fImg) {
    fImg->CheckInFullView(this);
    label(fImg->fGlass->GetName());
  }
}

FTW::NameBox::~NameBox()
{
  if(fImg) fImg->CheckOutFullView(this);
}

/**************************************************************************/

void FTW::NameBox::Absorb_Change(LID_t lid, CID_t cid)
{
  if((lid==1 && cid==1) || (lid==0 && cid==0)) {
    label(fImg->fGlass->GetName());
    redraw();
  }
}

void FTW::NameBox::ChangeImage(OS::ZGlassImg* new_img, bool keep_label)
{
  if(fImg) fImg->CheckOutFullView(this);
  fImg = new_img;
  if(fImg) {
    fImg->CheckInFullView(this);
    activate();
    label(fImg->fGlass->GetName());
  } else {
    if(keep_label) deactivate();
    else       	   label(0);
  }
  redraw();
}

/**************************************************************************/
/**************************************************************************/
// Selectors
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
// Instantiator Selector
/**************************************************************************/

namespace {
  struct Inst_SelID {
    FTW::Inst_Selector* fidsel;
    unsigned int      id;
    Inst_SelID(FTW::Inst_Selector* f, unsigned int i) : fidsel(f), id(i) {}
  };

  /**************************************************************************/

  void lid_sel_cb(Fl_Widget* w, Inst_SelID* ud) {
    ud->fidsel->set_lid(ud->id);
    ud->fidsel->set_cid(0);
  }

  void cid_sel_cb(Fl_Widget* w, Inst_SelID* ud) {
    ud->fidsel->set_cid(ud->id);
  }

  void fidsel_lid_cb(Fl_Button* b, Inst_SelID* ud) {
    map<UInt_t, string> res;
    for(GVNS::hLid2LSInfo_i i = GVNS::Lid2LSInfo.begin();
	i != GVNS::Lid2LSInfo.end(); ++i)
      {
	res[i->second.fLid] = i->second.fName;
      }

    Fl_Menu_Button menu(Fl::event_x_root(), Fl::event_y_root(), 0, 0, 0);
    Fl_SWM_Manager* mgr = Fl_SWM_Manager::search_manager(b);
    if(mgr) menu.textsize(mgr->cell_fontsize());

    list<Inst_SelID> datas;
    for(map<UInt_t, string>::iterator i = res.begin(); i!=res.end(); ++i) {
      datas.push_back( Inst_SelID(ud->fidsel, i->first) );
      menu.add(i->second.c_str(), 0, (Fl_Callback*)lid_sel_cb, &datas.back());
    }
    const Fl_Menu_Item* mi = menu.popup();
  }

  void fidsel_cid_cb(Fl_Button* b, Inst_SelID* ud) {
    GVNS::hLid2LSInfo_i i = GVNS::Lid2LSInfo.find(ud->fidsel->get_lid());
    if(i != GVNS::Lid2LSInfo.end()) {
      map<UInt_t, string> res;
      GVNS::LibSetInfo& lsi = i->second;
      for(GVNS::hCid2pCI_i j = lsi.Cid2pCI.begin();
	  j != lsi.Cid2pCI.end(); ++j)
	{
	  res[j->second->fFid.cid] = j->second->fClassName;
	}

      Fl_Menu_Button menu(Fl::event_x_root(), Fl::event_y_root(), 0, 0, 0);
      Fl_SWM_Manager* mgr = Fl_SWM_Manager::search_manager(b);
      if(mgr) menu.textsize(mgr->cell_fontsize());

      list<Inst_SelID> datas;
      for(map<UInt_t, string>::iterator j=res.begin(); j!=res.end(); ++j) {
	datas.push_back( Inst_SelID(ud->fidsel, j->first) );
	// Insert separators at ClassId breaks
	map<UInt_t, string>::iterator k = j; ++k;
	int flags = (k != res.end() && k->first - j->first > 1) ? FL_MENU_DIVIDER : 0;
	menu.add(j->second.c_str(), 0, (Fl_Callback*)cid_sel_cb, &datas.back(), flags);
      }
      const Fl_Menu_Item* mi = menu.popup();    
    }
  }

  void parent_fid_cb(Fl_Button* b, FTW::Inst_Selector* is) {
    // !!!!! this should go to instantiator menu!
    // get lid/cid
    // find cinfo; get parent
    // if parent -> set lid/cid
  }

  /**************************************************************************/

  void top_set_cb(Fl_Button* b, FTW::Bot_Selector* bs) {
    bs->set_top();
  }
}

/**************************************************************************/

FTW::Inst_Selector::Inst_Selector(Top_Selector* ts, Top_Selector::Type_e t) :
  Bot_Selector(ts, t), Fl_Group(0,0,23,2)
{
  wTop = new Fl_Light_Button(0,0,8,2,"Creator");
  wTop->selection_color(m_top->get_light_color());
  wTop->callback((Fl_Callback*)top_set_cb, this);
  wTop->labelfont(FL_HELVETICA_BOLD);

  Fl_Box* box;
  box = new Fl_Box(8,0,3,1,"Lid:");
  wLid = new Fl_Button(11,0,12,1);
  wLid->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  wLid->color(fl_gray_ramp(FL_NUM_GRAY-4));
  wLid->callback((Fl_Callback*)fidsel_lid_cb, new Inst_SelID(this, 0));

  box = new Fl_Box(8,1,3,1,"Cid:");
  wCid = new Fl_Button(11,1,12,1);
  wCid->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  wCid->color(fl_gray_ramp(FL_NUM_GRAY-4));
  wCid->callback((Fl_Callback*)fidsel_cid_cb, new Inst_SelID(this, 0));

  end();

  set_lid(1); set_cid(1);
}

FTW::Inst_Selector::~Inst_Selector() {
  delete (Inst_SelID*)(wLid->user_data());
  delete (Inst_SelID*)(wCid->user_data());
}

void FTW::Inst_Selector::set_lid(UInt_t l) {
  GVNS::hLid2LSInfo_i i = GVNS::Lid2LSInfo.find(l);
  if(i != GVNS::Lid2LSInfo.end()) {
    ((Inst_SelID*)wLid->user_data())->id = l;
    wLid->label( i->second.fName.c_str() );
    wLid->redraw();
  }
}

void FTW::Inst_Selector::set_cid(UInt_t c) {
  GVNS::ClassInfo* ci;
  if(c) {
    ci = GVNS::FindClassInfo(FID_t(get_lid(), c));
  } else {
    ci = GVNS::FindLibSetInfo(get_lid())->FirstClassInfo();
  }
    
  if(ci) {
    ((Inst_SelID*)wCid->user_data())->id = ci->fFid.cid;
    wCid->label( ci->fClassName.c_str() );
    wCid->redraw();
  }
}

void FTW::Inst_Selector::parent_fid() {
  // !!!! missing
}

UInt_t FTW::Inst_Selector::get_lid() {
  return ((Inst_SelID*)wLid->user_data())->id;
}

UInt_t FTW::Inst_Selector::get_cid() {
  return ((Inst_SelID*)wCid->user_data())->id;
}

/**************************************************************************/

void FTW::Inst_Selector::activate() {
  wTop->color(background_color + m_top->get_active_modcol());
  wTop->value(1);
  // Fl_Widget::activate();
  redraw();
}

void FTW::Inst_Selector::deactivate() {
  wTop->color(background_color);
  wTop->value(0);
  // Fl_Widget::deactivate();
  redraw();
}

int FTW::Inst_Selector::handle(int ev) {
  return Fl_Group::handle(ev);
}

/**************************************************************************/
// Nest Point/Mark Selector
/**************************************************************************/

namespace {
  void loc_sel_type_change_cb(Fl_Choice* c, FTW::Locator_Selector::LS_Type_e t) {
    FTW::Locator_Selector* npm = FTW::grep_parent<FTW::Locator_Selector*>(c);
    npm->set_type(t);
  }
}

FTW::Locator_Selector::Locator_Selector(Top_Selector* ts, Top_Selector::Type_e t) :
  Bot_Selector(ts, t), Fl_Group(0,0,18,2),
  m_type(LST_Undef), m_pref_nest(m_top->get_shell()->GetNest()), b_colored(false)
{
  wTop = new Fl_Light_Button(0,0,6,2,"Nest  ");
  wTop->type(0); wTop->when(FL_WHEN_RELEASE);
  wTop->selection_color(m_top->get_light_color());
  wTop->callback((Fl_Callback*)top_set_cb, this);
  wTop->labelfont(FL_HELVETICA_BOLD);

  wNestName = new NameBox((OS::ZGlassImg*)0,6,0,12,1);
  wNestName->label(m_pref_nest->fImg->fGlass->GetName());
  wNestName->deactivate();

  wType = new Fl_Choice(10,1,8,1, "Type:");
  wType->add("Undef", 0, (Fl_Callback*)loc_sel_type_change_cb, (void*)LST_Undef);
  wType->add("Point", 0, (Fl_Callback*)loc_sel_type_change_cb, (void*)LST_Point);
  wType->add("Mark",  0, (Fl_Callback*)loc_sel_type_change_cb, (void*)LST_Mark);
  wType->add("Own",   0, (Fl_Callback*)loc_sel_type_change_cb, (void*)LST_Own);

  end();

  set_type(LST_Undef);
}

/**************************************************************************/

void FTW::Locator_Selector::set_type(LS_Type_e type)
{
  switch(type) {
  case LST_Point:
    set_base((m_base ? m_base->nest : m_pref_nest)->RefPoint());
    break;
  case LST_Mark:
    set_base((m_base ? m_base->nest : m_pref_nest)->RefMark());
    break;
  default:
    clear_base();
    break;
  }
}

void FTW::Locator_Selector::set_base(Locator& loc) {
  if(b_colored) {
    if(m_base) m_base->mod_box_color(m_top->get_active_modcol(), false);
    loc.mod_box_color(m_top->get_active_modcol(), true);
  }

  LocatorConsumer::set_base(loc);
  m_pref_nest = loc.nest;

  wNestName->ChangeImage(m_base->nest->fImg);
  if(m_base->nest->IsPoint(loc))      wType->value(LST_Point);
  else if(m_base->nest->IsMark (loc)) wType->value(LST_Mark);
  else wType->value(LST_Own);
  redraw();
}

void FTW::Locator_Selector::clear_base() {
  if(b_colored) {
    if(m_base) m_base->mod_box_color(m_top->get_active_modcol(), false);
  }

  LocatorConsumer::clear_base();

  wNestName->ChangeImage(0, true);
  wType->value(LST_Undef);
  redraw();
}

void FTW::Locator_Selector::destroy_base() {
  clear_base();
  wNestName->ChangeImage(0, false);
}

void FTW::Locator_Selector::locator_change(Locator& loc) {
  assert(&loc == m_base); // testing LocatorConsumer ...
  if(b_colored) {
    loc.mod_old_box_color(m_top->get_active_modcol(), false);
    loc.mod_box_color(m_top->get_active_modcol(), true);
  }
}

/**************************************************************************/


void FTW::Locator_Selector::activate() {
  if(wTop->value() == 0) {
    if(m_base) {
      m_base->mod_box_color(m_top->get_active_modcol(), true);
    }
    b_colored = true;
    wTop->color(background_color + m_top->get_active_modcol());
    wTop->value(1);
    redraw();
  }
}

void FTW::Locator_Selector::deactivate() {
  if(wTop->value() == 1) {
    if(m_base) {
      m_base->mod_box_color(m_top->get_active_modcol(), false);
    }
    b_colored = false;
    wTop->color(background_color);
    wTop->value(0);
    redraw();
  }
}

int FTW::Locator_Selector::handle(int ev) {
  // !!! dnd missing
  return Fl_Group::handle(ev);
}

/**************************************************************************/
// /dev/null selector
/**************************************************************************/

FTW::Null_Selector::Null_Selector(Top_Selector* ts, Top_Selector::Type_e t) :
  Bot_Selector(ts, t), Fl_Group(0,0,8,2)
{
  wTop = new Fl_Light_Button(0,0,8,2,"/dev/null");
  wTop->selection_color(m_top->get_light_color());
  wTop->callback((Fl_Callback*)top_set_cb, this);
  wTop->labelfont(FL_HELVETICA_BOLD);

  end();
}

void FTW::Null_Selector::activate() {
  wTop->color(background_color + m_top->get_active_modcol());
  wTop->value(1);
  redraw();
}

void FTW::Null_Selector::deactivate() {
  wTop->color(background_color);
  wTop->value(0);
  redraw();
}

int FTW::Null_Selector::handle(int ev) {
  return Fl_Group::handle(ev);
}

/**************************************************************************/
// Source selector
/**************************************************************************/

FTW::Source_Selector::Source_Selector(FTW_Shell* s, int x, int y, int dw, const char* t) :
  Top_Selector(s), Fl_Pack (x,y,1,2)
{
  type(FL_HORIZONTAL);

  Fl_Box* b;

  b = new Fl_Box(FL_FLAT_BOX, 0,0,dw,2,t);
  b->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
  b->labelsize( labelsize() + 1 );
  b->labelfont(FL_HELVETICA_BOLD);
  b->color(background_color + get_active_modcol());

  b = new Fl_Box(separator_box, 0,0,1,2,0); b->color(separator_color);

  wLoc_Sel = new Locator_Selector(this, T_Locator);

  b = new Fl_Box(separator_box, 0,0,1,2,0); b->color(separator_color);

  wInst_Sel = new Inst_Selector(this, T_Inst);

  b = new Fl_Box(separator_box, 0,0,1,2,0); b->color(separator_color);

  //wListOpsSel = new ListOpsSelector(ListOpsSelector::T_Source);
  //wOnListOpRem = new Fl_Button(0,0,8,2, "On ListOps remove");
  //wOnListOpRem->align(FL_ALIGN_INSIDE | FL_ALIGN_WRAP | FL_ALIGN_TOP_LEFT);

  end();

  set_type(Top_Selector::T_Inst);
}

void FTW::Source_Selector::set_type(Type_e t)
{
  // could have Bot_sel* exsel
  switch(t) {

  case Top_Selector::T_Locator:
    wInst_Sel->deactivate();
    wLoc_Sel->activate();
    break;

  case Top_Selector::T_Inst:
    wLoc_Sel->deactivate();
    wInst_Sel->activate();
    break;

  default:
    cerr <<"FTW::Source_Selector::set_type wrong type "<< t <<endl;
    return;

  }

  m_cur_type = t;
}

/**************************************************************************/

bool FTW::Source_Selector::has_contents()
{
  switch (m_cur_type) {

  case Top_Selector::T_Locator: {
    if(wLoc_Sel->get_locator() == 0) return false;
    return wLoc_Sel->get_locator()->has_contents();
  }

  case Top_Selector::T_Inst:
    return true;

  default:
    return false;
  }
}

/**************************************************************************/

ZMIR* FTW::Source_Selector::generate_MIR_header(ZGlass* alpha, ZGlass* gamma)
{
  switch (m_cur_type) {

  case Top_Selector::T_Locator: {
    ID_t a = alpha->GetSaturnID();
    ID_t b = wLoc_Sel->get_locator()->get_contents();
    ID_t g = gamma ? gamma->GetSaturnID() : 0;
    return new ZMIR(a, b, g);
  }

  case Top_Selector::T_Inst: {
    // !!!! Need fixed queen option in Instantiator
    ZQueen* q = alpha->GetQueen();
    if(q==0) throw(string("Creator: target has no Queen"));
    return  q->S_InstantiateWAttach(alpha, gamma, wInst_Sel->get_lid(), wInst_Sel->get_cid());
  }

  default: {
    return 0;
  }

  }
}

/**************************************************************************/
// Sink Selector
/**************************************************************************/

FTW::Sink_Selector::Sink_Selector(FTW_Shell* s, int x, int y, int dw, const char* t) :
  Top_Selector(s), Fl_Pack (x,y,1,2)
{
  type(FL_HORIZONTAL);

  Fl_Box* b;

  b = new Fl_Box(FL_FLAT_BOX, 0,0,dw,2,t);
  b->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
  b->labelsize( labelsize() + 1 );
  b->labelfont(FL_HELVETICA_BOLD);
  b->color(background_color + get_active_modcol());

  b = new Fl_Box(separator_box, 0,0,1,2,0); b->color(separator_color);

  wLoc_Sel = new Locator_Selector(this, T_Locator);

  b = new Fl_Box(separator_box, 0,0,1,2,0); b->color(separator_color);

  wNull_Sel = new Null_Selector(this, T_DevNull);

  b = new Fl_Box(separator_box, 0,0,1,2,0); b->color(separator_color);

  //wListOpsSel = new ListOpsSelector(ListOpsSelector::T_Sink);
  //wOnListOpRem = new Fl_Button(0,0,8,2, "On ListOps remove");
  //wOnListOpRem->align(FL_ALIGN_INSIDE | FL_ALIGN_WRAP | FL_ALIGN_TOP_LEFT);

  end();

  set_type(Top_Selector::T_DevNull);
}

void FTW::Sink_Selector::set_type(Type_e t)
{
  switch(t) {

  case Top_Selector::T_Locator:
    wNull_Sel->deactivate();
    wLoc_Sel->activate();
    break;

  case Top_Selector::T_DevNull:
    wLoc_Sel->deactivate();
    wNull_Sel->activate();
    break;

  default:
    cerr <<"FTW::Sink_Selector::set_type wrong type "<< t <<endl;
    return;

  }

  m_cur_type = t;
}

/**************************************************************************/

void FTW::Sink_Selector::swallow_victim(ZGlass* beta)
{
  // Simplistic so far ... add to the list
  // listness of 

  static string _eh("FTW::Sink_Selector::swallow_victim ");

  switch (m_cur_type) {

  case Top_Selector::T_Locator: {
    Locator* target = wLoc_Sel->get_locator();
    if(!target->is_list()) throw(_eh + "sink is not a list");

    ID_t a = target->get_contents();
    ID_t b = beta->GetSaturnID();
    //ID_t g = gamma ? gamma->GetSaturnID() : 0;
    auto_ptr<ZMIR> mir(new ZMIR(a, b));
    GVNS::ContextMethodInfo* cmi = 
      target->get_class_info()->FindContextMethodInfo("Add");
    if(cmi) {
      (cmi->fooCCCreator)(mir->Message);
      target->leaf->fImg->fEye->Send(*mir);
    } else {
      throw(string(_eh + "Add" + " method not found in catalog"));
    }
    break;
  }

  case Top_Selector::T_DevNull:
  default:
    break;
  }
}

/**************************************************************************/
/**************************************************************************/
// Active Views
/**************************************************************************/
/**************************************************************************/

#include "Pupil.h"
#include <Glasses/PupilInfo.h>
#include "Eye.h"

/**************************************************************************/
// Pupil Am
/**************************************************************************/

void FTW::PupilAm::spawn_pupil(OS::ZGlassImg* img)
{
  PupilInfo* pi = dynamic_cast<PupilInfo*>( img->fGlass );
  if(pi && mImg2PupilMap.find(img) == mImg2PupilMap.end()) {
    img->AssertDefView();
    Pupil* poop = new Pupil(pi, img->fDefView);
    fImg->fEye->RegisterROARWindow(poop);
    mImg2PupilMap[img] = poop;
  }
}

/**************************************************************************/

FTW::PupilAm::PupilAm(FTW_Nest* n, OS::ZGlassImg* img) :
  OS::A_View(img), mNest(n)
{
  if(fImg) {
    fImg->CheckInFullView(this);
  }

  if(mNest->GetNestInfo()->GetSpawnPupils()) {
    ZList* l = dynamic_cast<ZList*>(fImg->fGlass);
    lpZGlass_t d; l->Copy(d);
    for(lpZGlass_i i=d.begin(); i!=d.end(); ++i) {
      spawn_pupil( fImg->fEye->DemanglePtr(*i) );
    }
  }
}

FTW::PupilAm::~PupilAm()
{
  // should close Pupils !!!!?? or get command from nest?
  if(fImg) fImg->CheckOutFullView(this);
}

/**************************************************************************/

void FTW::PupilAm::Absorb_ListAdd(OS::ZGlassImg* newimg, OS::ZGlassImg* before)
{
  if(mNest->GetNestInfo()->GetSpawnPupils()) {
    spawn_pupil(newimg);
  }
}

/**************************************************************************/
// Nest Am
/**************************************************************************/

void FTW::NestAm::spawn_nest(OS::ZGlassImg* img)
{
  NestInfo* ni = dynamic_cast<NestInfo*>( img->fGlass );
  if(ni && mImg2NestMap.find(img) == mImg2NestMap.end()) {
    FTW_Nest* nest = mShell->SpawnNest(img);
    mImg2NestMap[img] = nest;
    nest->show();
  }
}

/**************************************************************************/

FTW::NestAm::NestAm(FTW_Shell* s, OS::ZGlassImg* img) :
  OS::A_View(img), mShell(s)
{
  if(fImg) {
    fImg->CheckInFullView(this);
  }

  if(mShell->GetShellInfo()->GetSpawnNests()) {
    ZList* l = dynamic_cast<ZList*>(fImg->fGlass);
    lpZGlass_t d; l->Copy(d);
    for(lpZGlass_i i=d.begin(); i!=d.end(); ++i) {
      spawn_nest( fImg->fEye->DemanglePtr(*i) );
    }
  }
}

FTW::NestAm::~NestAm()
{
  // should close Nests !!!!?? or get command from nest?
  if(fImg) fImg->CheckOutFullView(this);
}

/**************************************************************************/

void FTW::NestAm::Absorb_ListAdd(OS::ZGlassImg* newimg, OS::ZGlassImg* before)
{
  if(mShell->GetShellInfo()->GetSpawnNests()) {
    spawn_nest(newimg);
  }
}
