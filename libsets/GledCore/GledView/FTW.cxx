// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.


#include "FTW.h"
#include "FTW_Shell.h"
#include "FTW_Nest.h"
#include "FTW_Leaf.h"
#include "FTW_Ant.h"
#include "FltkGledStuff.h"

#include <GledView/GledViewNS.h>

#include <Eye/Eye.h>
#include <Eye/Ray.h>
#include <Stones/ZMIR.h>
#include <Glasses/ZQueen.h>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/fl_ask.H>

namespace GNS  = GledNS;
namespace GVNS = GledViewNS;
namespace OS   = OptoStructs;
namespace FGS  = FltkGledStuff;

Fl_Color FTW::background_color       = (Fl_Color)0xbfbfbf00;
Fl_Color FTW::separator_color        = (Fl_Color)0xb2b28000;
Fl_Color FTW::postnamebox_color      = (Fl_Color)0xb280b200;
Fl_Color FTW::postnamebox_edit_color = (Fl_Color)0x80b2b200;
Fl_Color FTW::postnamebox_textcolor      = (Fl_Color)0x52205200;
Fl_Color FTW::postnamebox_edit_textcolor = (Fl_Color)0x20525200;

Fl_Color FTW::target_modcol = (Fl_Color)0x40000000;
Fl_Color FTW::source_modcol = (Fl_Color)0x00400000;
Fl_Color FTW::sink_modcol   = (Fl_Color)0x00004000;

Fl_Boxtype FTW::separator_box    = FL_THIN_UP_BOX; // FL_BORDER_BOX;
Fl_Boxtype FTW::postnamebox_box  = FL_EMBOSSED_BOX;

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
  if(ant)
    return ant->fToGlass ? ant->fToGlass->GetSaturnID() : 0;
  else
    return (leaf->fImg && leaf->fImg->fLens) ? leaf->fImg->fLens->GetSaturnID() : 0;
}

ZGlass* FTW::Locator::get_glass()
{ return ant ? ant->fToGlass : (leaf->fImg ? leaf->fImg->fLens : 0); }

OS::ZGlassImg* FTW::Locator::get_image()
{ return ant ? ant->GetToImg() : leaf->fImg; }

//ID_t FTW::Locator::get_id()
//{ return ant ? ant->fToImg->fLens->GetSaturnID() : leaf->fImg->fLens->GetSaturnID(); }

GNS::ClassInfo* FTW::Locator::get_class_info()
{ return ant ? ant->GetToImg()->GetCI() : leaf->fImg->GetCI(); }


ZGlass* FTW::Locator::get_leaf_glass()
{ return leaf->fImg->fLens; }

ID_t FTW::Locator::get_leaf_id()
{ return leaf->fImg->fLens->GetSaturnID(); }

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
  if (old_leaf)
  {
    if (old_ant)
      old_ant->modify_box_color(mod, on_p);
    else
      old_leaf->modify_box_color(mod, on_p);
  }
}

void FTW::Locator::mod_box_color(Fl_Color mod, bool on_p)
{
  if (leaf)
  {
    if (ant)
      ant->modify_box_color(mod, on_p);
    else
      leaf->modify_box_color(mod, on_p);
  }
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
  : Fl_Button(x,y,w,h), m_loc(l)
{ labeltype((Fl_Labeltype)GVNS::no_symbol_label); set_label(t); }

FTW::NameButton::NameButton(FTW_Leaf* leaf, int x, int y, int w, int h, const char* t)
  : Fl_Button(x,y,w,h), m_loc(L_Leaf)
{ labeltype((Fl_Labeltype)GVNS::no_symbol_label); set_label(t); }

FTW::NameButton::NameButton(FTW_Ant* ant, int x, int y, int w, int h, const char* t)
  : Fl_Button(x,y,w,h), m_loc(L_Ant)
{ labeltype((Fl_Labeltype)GVNS::no_symbol_label); set_label(t); }

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

void FTW::NameButton::set_nests_info_bar(FTW_Leaf* leaf, FTW_Ant* ant,
					 const char* prefix)
{
  if(leaf->fImg == 0) {
    leaf->GetNest()->SetInfoBar("<null> (list element not set)");
    return;
  }
  GledNS::ClassInfo* ci = leaf->fImg->GetCI();
  ZGlass* lens = leaf->fImg->fLens;
  if(ant == 0) {
    leaf->GetNest()->SetInfoBar(GForm("%s%s (\"%s\",\"%s\")",
				      prefix,
				      ci->fName.Data(),
				      lens->GetName(), lens->GetTitle()));
  } else {
    const char *glass = "<null>", *name = "", *title = "";
    if (ant->IsSet()) {
      glass = ant->GetToImg()->GetCI()->fName.Data();
      name  = ant->fToGlass->GetName();
      title = ant->fToGlass->GetTitle();
    }
    leaf->GetNest()->SetInfoBar(GForm("%s%s (\"%s\") [%s %s] -> %s (\"%s\",\"%s\")",
				      prefix,
				      ci->fName.Data(),
				      lens->GetName(),
				      ant->GetLinkInfo()->fType.Data(),
				      ant->GetLinkInfo()->fName.Data(),
				      glass, name, title));
  }

}

/**************************************************************************/

int FTW::NameButton::handle(int ev)
{
  static int x, y, dx, dy;
  FTW_Leaf* leaf = get_leaf();
  FTW_Ant*  ant  = get_ant();

  switch(ev) {

  case FL_ENTER: {
    if(leaf) {
      leaf->GetNest()->RefBelowMouse().set(leaf, ant);
      set_nests_info_bar(leaf, ant);
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

    case 2: {
      Fl::paste(*this);
      return 1;
    }

    case 3: {
      FTW_Shell *shell = leaf->GetNest()->GetShell();
      Locator loc(leaf->GetNest(), leaf, ant);
      if(loc.get_image() != 0)
	shell->FullMenu(loc.get_image(), Fl::event_x(), Fl::event_y());
      return 1;
    }
    } // end switch

    return 1;
  }


  case FL_DRAG: {
    Locator loc(leaf->GetNest(), leaf, ant);
    if(Fl::event_state(FL_BUTTON1)) {
      if(Fl::event_inside(this)) {
	dx = abs(Fl::event_x() - x); y = abs(Fl::event_y() - y);
	if(dx > w()/7) labelcolor(fl_color_cube(0,0,2));
	else	     labelcolor(fl_color_cube(2,0,0));
	value(1);
      } else {
	// we have just left the widget ... initiate dnd
	const char* text = GForm("%u", loc.get_contents());
        leaf->GetNest()->GetShell()->X_SetSource(loc.get_image());
        Fl::copy(text, strlen(text), 0);
        Fl::dnd();
	labelcolor(loc.get_image() ? FL_BLACK : FL_DARK_RED);
	value(0);
      }
    } else {
      labelcolor(loc.get_image() ? FL_BLACK : FL_DARK_RED);
      value(0);
    }
    redraw();
    return 1;
  }

  case FL_RELEASE: {
    Locator loc(leaf->GetNest(), leaf, ant);
    if(Fl::event_button() == 1 && Fl::event_inside(this)) {
      if(Fl::event_clicks() == 1 && loc.get_glass() != 0) {
	FTW_Shell *shell = leaf->GetNest()->GetShell();
        shell->SpawnMTW_View(loc.get_image(), true, true);
        leaf->GetNest()->RefPoint().revert();
        Fl::event_clicks(0);
      } else {
	if(dx > w()/7) leaf->GetNest()->RefMark().set(leaf, ant);
	else	       leaf->GetNest()->RefPoint().set(leaf, ant);
      }
    }
    labelcolor(loc.get_image() ? FL_BLACK : FL_DARK_RED);
    value(0);
    redraw();
    return 1;
  }

  case FL_DND_ENTER: {
    if(ant == 0) {
      set_nests_info_bar(leaf, ant, "Dnd - Push to list: ");
    } else {
      set_nests_info_bar(leaf, ant, "Dnd - Set link: ");
    }
    return 1;
  }

  case FL_DND_RELEASE: {
    return (Fl::belowmouse() == this) ? 1 : 0;
  }

  case FL_DND_LEAVE: {
    return 1;
  }

  case FL_PASTE: {
    FTW_Shell* shell = leaf->GetNest()->GetShell();
    try {
      Locator me(leaf->GetNest(), leaf, ant);
      if(ant == 0) {
        shell->X_Add(me);
      } else {
        if(shell->GetSource()->has_contents())
	  shell->X_SetLinkOrElement(me);
        else
          shell->X_ClearLinkOrElement(me);
      }
    }
    catch(Exc_t& exc) {
      shell->Message(exc.Data(), ISerror);
    }
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
  labeltype((Fl_Labeltype)GVNS::no_symbol_label);
  if(fImg) {
    label(fImg->fLens->GetName());
  }
}

/**************************************************************************/

void FTW::NameBox::AbsorbRay(Ray& ray)
{
  if(ray.IsBasicChange()) {
    label(fImg->fLens->GetName());
    redraw();
  }
}

void FTW::NameBox::ChangeImage(OS::ZGlassImg* new_img)
{
  SetImg(new_img);
  if(fImg) {
    activate();
    copy_label(fImg->fLens->GetName());
  } else {
    deactivate();
    label("<null>");
  }
  redraw();
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
// ListDesignator
/**************************************************************************/

FTW::ListDesignator::ListDesignator(bool editp) :
  Fl_Input(0, 0, 1, 1),
  m_editable    (editp),
  m_edit_active (false)
{
  if(m_editable) textcolor(FTW::postnamebox_edit_textcolor);
  else           textcolor(FTW::postnamebox_textcolor);
  color(FL_LIGHT2);
}

void FTW::ListDesignator::edit_active(bool ea)
{
  if(m_edit_active == ea || !m_editable) return;
  if(m_edit_active) {
    textcolor(FTW::postnamebox_edit_textcolor);
    color(FL_LIGHT2);
  } else {
    textcolor(FL_BLACK);
    color(FL_WHITE);
  }
  m_edit_active = ea;
}

int FTW::ListDesignator::handle(int ev)
{
  switch (ev) {

  case FL_PUSH:
    if(Fl::event_button() == 2) {
      Fl::paste(*this);
      return 1;
    }
    break;

  case FL_DND_ENTER:
    return 1;

  case FL_DND_RELEASE: {
    return (Fl::belowmouse() == this) ? 1 : 0;
  }

  case FL_DND_LEAVE: {
    // restore cursor
    return 1;
  }

  case FL_PASTE: {
    FTW_Shell *shell = FGS::grep_shell(parent());
    if(shell) {
      ID_t source_id = shell->GetSource()->get_contents();
      ID_t pasted_id = (ID_t) strtoul(Fl::event_text(), 0, 0);
      if(source_id == pasted_id) {
        FTW_Leaf* l = FGS::grep_parent<FTW_Leaf*>(parent());
        AList*        list = l->GetParent()->fImg->GetList();
        ZGlass*       lens = shell->DemangleID2Lens(source_id);
        AList::ElRep elrep = l->GetElRep();
        auto_ptr<ZMIR> mir
          (list->MkMir_SetElement(lens, elrep));
        shell->Send(*mir);
        return 1;
      }
    }
    break;
  }
  }

  if(m_edit_active)
    return Fl_Input::handle(ev);
  return 0;
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
    GNS::lpLSI_t ls_list;
    GNS::ProduceLibSetInfoList(ls_list);

    Fl_Menu_Button menu(Fl::event_x_root(), Fl::event_y_root(), 0, 0, 0);
    Fl_SWM_Manager* mgr = Fl_SWM_Manager::search_manager(b);
    if(mgr) menu.textsize(mgr->cell_fontsize());
    list<Inst_SelID> datas;

    for(GNS::lpLSI_i lsi=ls_list.begin(); lsi!=ls_list.end(); ++lsi) {
      datas.push_back( Inst_SelID(ud->fidsel, (*lsi)->fLid) );
      menu.add((*lsi)->fName.Data(), 0, (Fl_Callback*)lid_sel_cb, &datas.back());
    }
    menu.popup();
  }

  void fidsel_cid_cb(Fl_Button* b, Inst_SelID* ud) {
    GNS::LibSetInfo* lsi = GNS::FindLibSetInfo(ud->fidsel->get_lid());
    if(lsi == 0) return; // !!!! scream

    map<CID_t, TString> res;
    for(GNS::hCid2pCI_i j = lsi->Cid2CInfo.begin();
	j != lsi->Cid2CInfo.end(); ++j)
      {
	res[j->second->fFid.fCid] = j->second->fName;
      }

    Fl_Menu_Button menu(Fl::event_x_root(), Fl::event_y_root(), 0, 0, 0);
    Fl_SWM_Manager* mgr = Fl_SWM_Manager::search_manager(b);
    if(mgr) menu.textsize(mgr->cell_fontsize());

    list<Inst_SelID> datas;
    for(map<CID_t, TString>::iterator j=res.begin(); j!=res.end(); ++j) {
      datas.push_back( Inst_SelID(ud->fidsel, j->first) );
      // Insert separators at ClassId breaks
      map<CID_t, TString>::iterator k = j; ++k;
      int flags = (k != res.end() && k->first - j->first > 1) ? FL_MENU_DIVIDER : 0;
      menu.add(j->second.Data(), 0, (Fl_Callback*)cid_sel_cb, &datas.back(), flags);
    }
    menu.popup();

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

  new Fl_Box(8,0,3,1,"Lid:");
  wLid = new Fl_Button(11,0,12,1);
  wLid->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  wLid->color(fl_gray_ramp(FL_NUM_GRAY-4));
  wLid->callback((Fl_Callback*)fidsel_lid_cb, new Inst_SelID(this, 0));

  new Fl_Box(8,1,3,1,"Cid:");
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
  GNS::LibSetInfo* lsi = GNS::FindLibSetInfo(l);
  if(lsi != 0) {
    ((Inst_SelID*)wLid->user_data())->id = l;
    wLid->label( lsi->fName.Data() );
    wLid->redraw();
  }
}

void FTW::Inst_Selector::set_cid(UInt_t c) {
  GNS::ClassInfo* ci;
  if(c) {
    ci = GNS::FindClassInfo(FID_t(get_lid(), c));
  } else {
    ci = GNS::FindLibSetInfo(get_lid())->FirstClassInfo();
  }

  if(ci) {
    ((Inst_SelID*)wCid->user_data())->id = ci->fFid.fCid;
    wCid->label( ci->fName.Data() );
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
// Nest Locator Selector
/**************************************************************************/

namespace {
  void loc_sel_type_change_cb(Fl_Choice* c, FTW::Locator_Selector::LS_Type_e t) {
    FTW::Locator_Selector* npm = FGS::grep_parent<FTW::Locator_Selector*>(c);
    npm->set_type(t);
  }
}

FTW::Locator_Selector::Locator_Selector(Top_Selector* ts, Top_Selector::Type_e t) :
  Bot_Selector(ts, t), Fl_Group(0,0,18,2),
  m_type(LST_Undef), b_colored(false)
{
  wTop = new Fl_Light_Button(0,0,6,2,"Nest  ");
  wTop->type(0); wTop->when(FL_WHEN_RELEASE);
  wTop->selection_color(m_top->get_light_color());
  wTop->callback((Fl_Callback*)top_set_cb, this);
  wTop->labelfont(FL_HELVETICA_BOLD);

  wNestName = new NameBox((OS::ZGlassImg*)0,6,0,12,1);
  wNestName->deactivate();

  wType = new Fl_Choice(10,1,8,1, "Type:");
  wType->add("Undef", 0, (Fl_Callback*)loc_sel_type_change_cb, (void*)LST_Undef);
  wType->add("Point", 0, (Fl_Callback*)loc_sel_type_change_cb, (void*)LST_Point);
  wType->add("Mark",  0, (Fl_Callback*)loc_sel_type_change_cb, (void*)LST_Mark);

  end();

  set_type(LST_Undef);
}

/**************************************************************************/

void FTW::Locator_Selector::set_type(LS_Type_e type)
{
  switch(type) {
  case LST_Point:
    if(m_base) set_base(m_base->nest->RefPoint());
    else       clear_base();
    break;
  case LST_Mark:
    if(m_base) set_base(m_base->nest->RefMark());
    else       clear_base();
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

  wNestName->ChangeImage(m_base->nest->fImg);
  if(m_base->nest->IsPoint(loc))      wType->value(LST_Point);
  else if(m_base->nest->IsMark (loc)) wType->value(LST_Mark);
  else wType->value(LST_Undef);
  redraw();
}

void FTW::Locator_Selector::clear_base() {
  if(b_colored) {
    if(m_base) m_base->mod_box_color(m_top->get_active_modcol(), false);
  }

  LocatorConsumer::clear_base();

  wNestName->ChangeImage(0);
  wType->value(LST_Undef);
  redraw();
}

void FTW::Locator_Selector::destroy_base() {
  clear_base();
  wNestName->ChangeImage(0);
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
// Direct bot selector
/**************************************************************************/

FTW::Direct_Selector::Direct_Selector(Top_Selector* ts, Top_Selector::Type_e t) :
  Bot_Selector(ts, t), Fl_Group(0,0,18,2)
{
  wTop = new Fl_Light_Button(0,0,6,2,"Direct");
  wTop->selection_color(m_top->get_light_color());
  wTop->callback((Fl_Callback*)top_set_cb, this);
  wTop->labelfont(FL_HELVETICA_BOLD);

  wNameBox = new FGS::LensNameBox(0, 6, 0, 12, 1);
  wNameBox->box(FL_BORDER_BOX);

  end();
}

void FTW::Direct_Selector::activate() {
  wTop->color(background_color + m_top->get_active_modcol());
  wTop->value(1);
  redraw();
}

void FTW::Direct_Selector::deactivate() {
  wTop->color(background_color);
  wTop->value(0);
  redraw();
}

OS::ZGlassImg* FTW::Direct_Selector::get_img()
{
  return wNameBox->fImg;
}

void FTW::Direct_Selector::set_img(OS::ZGlassImg* img)
{
  wNameBox->ChangeImage(img);
}

ID_t FTW::Direct_Selector::get_id()
{
  return wNameBox->fImg ? wNameBox->fImg->fLens->GetSaturnID() : 0;
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
  wDir_Sel = new Direct_Selector(this, T_Direct);

  end();

  wCurrent = 0;
  set_type(Top_Selector::T_Inst);
}

void FTW::Source_Selector::set_type(Type_e t)
{
  static const Exc_t _eh("Source_Selector::set_type ");

  Bot_Selector* cur = 0;
  switch(t) {
  case Top_Selector::T_Locator: cur = wLoc_Sel;  break;
  case Top_Selector::T_Inst:    cur = wInst_Sel; break;
  case Top_Selector::T_Direct:  cur = wDir_Sel;  break;
  default:
    cerr << _eh << "unknown type "<< t <<endl;
    return;
  }

  if(wCurrent != 0) { wCurrent->deactivate(); }
  wCurrent = cur;
  wCurrent->activate();
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

  case Top_Selector::T_Direct:
    return wDir_Sel->get_img() != 0;

  default:
    return false;
  }
}

ID_t FTW::Source_Selector::get_contents()
{
  switch (m_cur_type) {

  case Top_Selector::T_Locator: {
    if(wLoc_Sel->get_locator() == 0) return 0;
    return wLoc_Sel->get_locator()->get_contents();
  }

  case Top_Selector::T_Inst:
    return 0;

  case Top_Selector::T_Direct:
    return wDir_Sel->get_img() ? wDir_Sel->get_img()->fLens->GetSaturnID() : 0;

  default:
    return 0;
  }
}

/**************************************************************************/

void FTW::Source_Selector::fix_MIR_beta(auto_ptr<ZMIR>& mir)
{
  switch (m_cur_type) {

  case Top_Selector::T_Locator:
    mir->fBetaID = get_contents();
    break;

  case Top_Selector::T_Inst: {
    // !!! Need fixed queen option in Instantiator
    static const Exc_t _eh("Creator: ");
    if(mir->fAlpha == 0)
      throw(_eh + "alpha not set, can't deduce queen.");
    ZQueen* q = mir->fAlpha->GetQueen();
    if(q==0)
      throw(_eh + "target has no Queen");

    ZMIR *m = q->S_InstantiateWAttach(wInst_Sel->get_lid(),
				      wInst_Sel->get_cid());
    m->ChainMIR(mir.get());
    mir.reset(m);

    break;
  }

  case Top_Selector::T_Direct:
    mir->fBetaID = wDir_Sel->get_id();
    break;

  default:
    break;

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
  wDir_Sel = new Direct_Selector(this, T_Direct);

  //wListOpsSel = new ListOpsSelector(ListOpsSelector::T_Sink);
  //wOnListOpRem = new Fl_Button(0,0,8,2, "On ListOps remove");
  //wOnListOpRem->align(FL_ALIGN_INSIDE | FL_ALIGN_WRAP | FL_ALIGN_TOP_LEFT);

  end();

  wCurrent = 0;
  set_type(Top_Selector::T_DevNull);
}

void FTW::Sink_Selector::set_type(Type_e t)
{
  static const Exc_t _eh("Sink_Selector::set_type ");

  Bot_Selector* cur = 0;
  switch(t) {
  case Top_Selector::T_Locator: cur = wLoc_Sel;  break;
  case Top_Selector::T_DevNull: cur = wNull_Sel; break;
  case Top_Selector::T_Direct:  cur = wDir_Sel;  break;
  default:
    cerr << _eh << "unknown type "<< t <<endl;
    return;
  }

  if(wCurrent != 0) { wCurrent->deactivate(); }
  wCurrent = cur;
  wCurrent->activate();
  m_cur_type = t;
}

/**************************************************************************/

void FTW::Sink_Selector::swallow_victim(ZGlass* beta)
{
  // Simplistic so far ... add to the list
  // listness of

  static const Exc_t _eh("FTW::Sink_Selector::swallow_victim ");

  switch (m_cur_type) {

  case Top_Selector::T_Locator: {
    Locator* target = wLoc_Sel->get_locator();
    if(!target->is_list()) throw(_eh + "sink is not a list.");

    ID_t a = target->get_contents();
    ID_t b = beta->GetSaturnID();
    //ID_t g = gamma ? gamma->GetSaturnID() : 0;
    auto_ptr<ZMIR> mir(new ZMIR(a, b));
    GNS::MethodInfo* cmi =
      target->get_class_info()->FindMethodInfo("Add", true);
    if(cmi) {
      cmi->ImprintMir(*mir);
      target->leaf->fImg->fEye->Send(*mir);
    } else {
      throw(_eh + "Add" + " method not found in catalog.");
    }
    break;
  }

  case Top_Selector::T_Direct: {
    OS::ZGlassImg* aimg = wDir_Sel->get_img();
    if(aimg == 0) return;
    if(!aimg->IsList()) throw(_eh + "sink is not a list");

    ID_t a = aimg->fLens->GetSaturnID();
    ID_t b = beta ? beta->GetSaturnID() : 0;
    //ID_t g = gamma ? gamma->GetSaturnID() : 0;
    auto_ptr<ZMIR> mir(new ZMIR(a, b));
    GNS::MethodInfo* cmi =
      aimg->GetCI()->FindMethodInfo("Add", true);
    if(cmi) {
      cmi->ImprintMir(*mir);
      aimg->fEye->Send(*mir);
    } else {
      throw(_eh + "Add method not found in catalog.");
    }
    break;
  }

  case Top_Selector::T_DevNull:
  default:
    break;
  }
}
