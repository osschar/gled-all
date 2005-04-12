// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "FltkGledStuff.h"
#include "Eye.h"
#include "FTW_Shell.h"

#include "MCW_View.h"

#include <Glasses/ZList.h>
#include <Net/Ray.h>
#include <Stones/ZMIR.h>

#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/fl_draw.H>

namespace FGS  = FltkGledStuff;
namespace OS   = OptoStructs;
namespace GNS  = GledNS;
namespace GVNS = GledViewNS;

/**************************************************************************/
// Fl helpers
/**************************************************************************/

FTW_Shell* FGS::grep_shell(Fl_Widget *w)
{
  FTW_SubShell* c = grep_parent<FTW_SubShell*>(w);
  if(c) return c->GetShell();
  return grep_parent<FTW_Shell*>(w);
}

FTW_Shell* FGS::grep_shell_or_die(Fl_Widget *w, const string& _eh)
{
  FTW_Shell* s = grep_shell(w);
  if(s == 0) throw(_eh + "can not reach FTW_Shell.");
  return s;
}

/**************************************************************************/

int FGS::swm_generick_width(string& str, int cell_w, float extra)
{
  if(cell_w) {
    int w=0, h=0;
    fl_measure(str.c_str(), w, h);
    float f = float(w)/cell_w + extra;
    w = int(f);
    return w + ((f-w < 0.1) ? 0 : 1);
  } else {
    return str.size();
  }
}

int FGS::swm_label_width(string& str, int cell_w)
{ return swm_generick_width(str, cell_w, 1.2); }

int FGS::swm_string_width(string& str, int cell_w)
{ return swm_generick_width(str, cell_w, 0.2); }

/**************************************************************************/
// PackEntryCollapsor
/**************************************************************************/

// Collapses/expands next widget in the group (hopefully vertical Fl_Pack).
// Does not work very well with resizable windows.

FGS::PackEntryCollapsor::PackEntryCollapsor(const char* t) :
  Fl_Group(0,0,4,1), mColWid(0)
{
  fBut = new Fl_Button(0,0,2,1, "@#>");
  fBut->labeltype(FL_SYMBOL_LABEL);
  fBut->callback((Fl_Callback*)cb_collexp, this);
  fBut->color(fl_rgb_color(200,220,200));

  fBox = new Fl_Box(2,0,2,1, t);
  fBox->box(FL_EMBOSSED_BOX);
  if(fBox->labelfont() < FL_BOLD)
    fBox->labelfont(fBox->labelfont() + FL_BOLD);
  fBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  fBox->color(fl_rgb_color(200,220,200));

  end();
  resizable(fBox);
}

int FGS::PackEntryCollapsor::collexp(bool resize_p)
{
  Fl_Group* pg = parent(); if(pg == 0) return 0;

  if(mColWid == 0) {
    int i = pg->find(this);
    if(i >= pg->children() - 1) return 0;
    mColWid = pg->child(i+1);
  }

  int dh = 0;
  if(mColWid->visible()) {
    mColWid->hide();
    fBut->label("@#>[]");
    dh = -mColWid->h();
  } else {
    mColWid->show();
    fBut->label("@#>");
    dh = mColWid->h();
  }
  
  if(resize_p) {
    pg->init_sizes();
    pg->redraw();
    Fl_Window* win = dynamic_cast<Fl_Window*>(pg->parent());
    if(win) {
      Fl_Widget* res = win->resizable();
      win->resizable(0);
      win->position(win->x(), win->y() + dh/2);
      win->size(win->w(), pg->h()+dh);
      win->redraw();
      win->resizable(res);
    }
  }

  return dh;
}

/**************************************************************************/
// FGS::LensNameBox
/**************************************************************************/

FGS::LensNameBox::LensNameBox(OS::ZGlassImg* i, int x, int y, int w, int h, const char* t) :
  OS::A_View(0), Fl_Box(x,y,w,h,t)
{
  labeltype((Fl_Labeltype)GVNS::no_symbol_label);
  color(fl_rgb_color(224,224,224));
  ChangeImage(i);
}

/**************************************************************************/

void FGS::LensNameBox::AbsorbRay(Ray& ray)
{
  using namespace RayNS;
  if(ray.IsBasicChange()) {
    auto_label(); return;
  }
  if(ray.fRQN == RQN_death) {
    ChangeImage(0); return;
  }
}

/**************************************************************************/

void FGS::LensNameBox::ImagePasted(OS::ZGlassImg* new_img)
{
  ChangeImage(new_img);
}

void FGS::LensNameBox::ChangeImage(OS::ZGlassImg* new_img)
{
  static const string _eh("FGS::LensNameBox::ChangeImage ");

  if(new_img && !fFID.is_null() && !GNS::IsA(new_img->fGlass, fFID)) {
    FTW_Shell* shell = grep_shell_or_die(parent(), _eh);
    shell->Message
      (GForm("%sargument '%s::%s' is not of required type '%s'.", _eh.c_str(),
	     new_img->fClassInfo->fName.c_str(), new_img->fGlass->GetName(),
	     GNS::FindClassInfo(fFID)->fName.c_str())
      );
    return;
  }

  SetImg(new_img);
  auto_label();
}

/**************************************************************************/

void FGS::LensNameBox::auto_label()
{
  if(fImg) {
    GNS::ClassInfo*   ci = fImg->fClassInfo;
    GNS::LibSetInfo* lsi = GNS::FindLibSetInfo(ci->fFid.lid);
    mToName = fImg->fGlass->GetName();
    set_tooltip(GForm("%s::%s [%d]", lsi->fName.c_str(), ci->fName.c_str(),
		      fImg->fGlass->GetSaturnID()));
  } else {
    mToName = "<null>";
    set_tooltip(0);
  }
  redraw();
}
/**************************************************************************/

void FGS::LensNameBox::draw()
{
  static const string _eh("LensNameBox::draw ");

  draw_box();

  string text;
  if(align() & FL_ALIGN_INSIDE) {
    text = GForm("%s: %s", label(), mToName.c_str());
  } else {
    text = mToName;
    draw_label();
  }

  int X = x() + Fl::box_dx(box()) + 3, Y = y() + Fl::box_dy(box());
  int W = w() - Fl::box_dw(box()) - 3, H = h() - Fl::box_dh(box());
  fl_color(FL_BLACK);
  fl_font(labelfont(), labelsize());
  fl_push_clip(X, Y, W, H);
  fl_draw(text.c_str(), X, Y, W, H, FL_ALIGN_LEFT, 0, 0);
  fl_pop_clip();	
}

/**************************************************************************/

namespace {
  void clear_cb(Fl_Widget* w, FGS::LensNameBox* ud) { ud->Clear(); }
}

int FGS::LensNameBox::handle(int ev)
{
  static const string _eh("FGS::LensNameBox::handle ");

  switch(ev) {

  case FL_PUSH: {
    FTW_Shell *shell = grep_shell(parent());
    if(shell == 0) return 0;
    switch (Fl::event_button()) {
    case 1: return 1;
    case 2: Fl::paste(*this); return 1;
    case 3:
      if(fImg) {
	Fl_Menu_Button menu(Fl::event_x_root(), Fl::event_y_root(), 0, 0, 0);
	menu.textsize(shell->cell_fontsize());
	FTW_Shell::mir_call_data_list mcdl;
	menu.add("Clear", 0, (Fl_Callback*)clear_cb, this, FL_MENU_DIVIDER);
	shell->FillLensMenu(fImg, menu, mcdl, "");
	shell->FillShellVarsMenu(fImg, menu, mcdl, "");

	menu.popup();
      }
      return 1;
    }
    break;
  }

  case FL_DRAG: {
    if(Fl::event_state(FL_BUTTON1)) {
      if( ! Fl::event_inside(this) && fImg != 0 ) {
	FTW_Shell* shell = grep_shell_or_die(parent(), _eh);
	ID_t id          = fImg->fGlass->GetSaturnID();
	const char* text = GForm("%u", id);
	shell->X_SetSource(fImg);
	Fl::copy(text, strlen(text), 0);
	Fl::dnd();
      }
      return 1;
    }
    break;
  }

  case FL_RELEASE:
    if(fImg && Fl::event_button() == 1 && Fl::event_inside(this) &&
       Fl::event_clicks() == 1) 
      {
	Fl::event_clicks(0);
	FTW_Shell* shell = grep_shell_or_die(parent(), _eh);
	shell->SpawnMTW_View(fImg);
      }
    return 1;

  case FL_DND_ENTER: {
    FTW_Shell *shell = grep_shell(parent());
    if(shell == 0) return 0;
    // could check if valid type, change cursor
    return 1;
  }

  case FL_DND_RELEASE: {
    return (Fl::belowmouse() == this) ? 1 : 0;
  }

  case FL_DND_LEAVE: {
    // restore cursor
    return 1;
  }

  case FL_PASTE: {
    FTW_Shell *shell = grep_shell(parent());
    if(shell == 0) return 0;
    ID_t source_id = shell->GetSource()->get_contents();
    ImagePasted(shell->DemangleID(source_id));
    return 1;
  }
  } // end switch(ev)

  return Fl_Box::handle(ev);
}

/**************************************************************************/
// LensRepNameBox
/**************************************************************************/

FGS::LensRepNameBox::LensRepNameBox(OS::ZGlassImg* i, int x, int y, int w, int h, const char* t) :
  LensNameBox(i,x,y,w,h,t)
{}

void FGS::LensRepNameBox::ImagePasted(OS::ZGlassImg* new_img)
{
  FTW_Shell *shell = grep_shell(parent());
  if(shell->GetSource()->has_contents()) {
    // Here missing FID check.
    GNS::MethodInfo* cmi = fImg->fClassInfo->FindMethodInfo("Add", true);
    if(cmi) {
      auto_ptr<ZMIR> mir (shell->GetSource()->generate_MIR(cmi, fImg->fGlass));
      shell->Send(*mir);
    }
  }
}

/**************************************************************************/
// LinkNameBox
/**************************************************************************/

FGS::LinkNameBox::LinkNameBox(OS::ZLinkDatum* ld, int x, int y, int w, int h, const char* t) :
  OS::ZLinkView(ld), LensNameBox(ld->GetToImg(),x,y,w,h,t)
{
  box(FL_UP_BOX);
  color(fl_rgb_color(200, 200, 220));

  Update();
}

/**************************************************************************/

void FGS::LinkNameBox::Update()
{
  ZLinkView::Update();
  ChangeImage(GetToImg());
}

/**************************************************************************/

void FGS::LinkNameBox::ImagePasted(OptoStructs::ZGlassImg* new_img)
{
  static const string _eh("LinkNameBox::ImagePasted ");

  FTW_Shell* shell = grep_shell_or_die(parent(), _eh);
  GNS::MethodInfo* mi = GetLinkInfo()->fSetMethod;

  auto_ptr<ZMIR> mir (shell->GetSource()->generate_MIR(mi, fLinkDatum->fImg->fGlass));
  fLinkDatum->fImg->fEye->Send(*mir);
}

void FGS::LinkNameBox::Clear()
{
  ZMIR mir(fLinkDatum->fImg->fGlass->GetSaturnID(), 0);
  GetLinkInfo()->fSetMethod->ImprintMir(mir);
  fLinkDatum->fImg->fEye->Send(mir);
}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
// MenuBox
/**************************************************************************/

void FGS::MenuBox::_init()
{
  box((Fl_Boxtype)GVNS::menubar_box);
  align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  menu_item(fMenuItem);
}

FGS::MenuBox::MenuBox(int x, int y, int w, int h, const char* t) :
  Fl_Button(x,y,w,h,t), fMenuItem(0), fMenuButton(0)
{ _init(); }

FGS::MenuBox::MenuBox(Fl_Menu_Item* mi, int w, int h, const char* t) :
  Fl_Button(0,0,w,h,t), fMenuItem(mi), fMenuButton(0)
{ _init(); }

FGS::MenuBox::~MenuBox()
{
  delete fMenuButton;
}

/**************************************************************************/

void FGS::MenuBox::menu_item(Fl_Menu_Item* mi)
{
  delete fMenuButton;
  fMenuButton = 0;
  fMenuItem = mi;
  if(fMenuItem) {
    fMenuButton = new Fl_Menu_Button(0,0,0,0);
    fMenuButton->hide();
    fMenuButton->parent(parent());
    fMenuButton->menu(fMenuItem);
    fMenuButton->box(FL_BORDER_BOX);
  }
}

int FGS::MenuBox::handle(int ev)
{
  static const string _eh("FGS::MenuBox::handle ");

  if(fMenuButton == 0) return 0;

  if(ev == FL_PUSH && Fl::event_button() == 1) {
    FTW_Shell* shell = grep_shell_or_die(parent(), _eh);
    fMenuButton->resize(x(), y(), w(), h() - Fl::box_dh(box()));
    fMenuButton->textsize(shell->cell_fontsize());
    fMenuButton->popup();
    return 1;
  }

  if(ev == FL_SHORTCUT) {
    return fMenuButton->handle(ev);
  }

  return 0;
}

/**************************************************************************/
// LensChoiceMenuBox
/**************************************************************************/

FGS::LensChoiceMenuBox::LensChoiceMenuBox(OS::ZGlassImg* i,
				     int x, int y, int w, int h, const char* t) :
  A_View(0), Fl_Box(x,y,w,h,t),
  mAlphaImg(i), mSrcLinkDatum(0), mMInfo(0)
{
  align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
}

void FGS::LensChoiceMenuBox::AbsorbRay(Ray& ray)
{
  if(ray.fRQN == RayNS::RQN_death) {
    SetSrcImg(0);
    return;
  }
}

void FGS::LensChoiceMenuBox::EmitMir(ID_t beta_id)
{
  static const string _eh("LensChoiceMenuBox::EmitMir ");

  FTW_Shell* shell = grep_shell_or_die(parent(), _eh);
  auto_ptr<ZMIR> mir( new ZMIR(mAlphaImg->fGlass->GetSaturnID(), beta_id) );
  mMInfo->ImprintMir(*mir);
  mMInfo->FixMirBits(*mir, shell->GetSaturnInfo());
  shell->Send(*mir);
}

/**************************************************************************/

namespace {
  void lcmb_select_cb(Fl_Menu_Button* m, void* ud)
  { ((FGS::LensChoiceMenuBox*)m->user_data())->EmitMir((ID_t)ud); }
}

void FGS::LensChoiceMenuBox::fill_menu(ZList* list, Fl_Menu_Button& menu,
				       string prefix)
{
  lpZGlass_t l; list->Copy(l);
  for(lpZGlass_i i=l.begin(); i!=l.end(); ++i) {

    string name    = prefix + (*i)->GetName();
    ZList* clist   = dynamic_cast<ZList*>(*i);
    bool   list_ok = clist && clist->Size();

    if(GledNS::IsA(*i, mSrcFid)) {
      menu.add(name.c_str(), 0, (Fl_Callback*)lcmb_select_cb,
	       (void*)((*i)->GetSaturnID()), list_ok ? FL_SUBMENU : 0);
    }
    if(list_ok) {
      fill_menu(clist, menu, name + "/");
    }
  }
}

int FGS::LensChoiceMenuBox::handle(int ev)
{
  static const string _eh("FGS::LensChoiceMenuBox::handle ");

  if(ev == FL_PUSH && Fl::event_button() == 1) {
    if(mMInfo == 0) return 0;

    FTW_Shell* shell = grep_shell_or_die(parent(), _eh);

    ZList* zlist = get_src_list(shell);
    if(zlist == 0) return 0;

    Fl_Menu_Button m(x(), y(), w(), h() - Fl::box_dh(box()));
    m.parent(parent());
    m.user_data(this);
    m.textsize(shell->cell_fontsize());

    fill_menu(zlist, m, "");

    m.popup();
    return 1;
  }

  return Fl_Box::handle(ev);
}

/**************************************************************************/

ZList* FGS::LensChoiceMenuBox::get_src_list(FTW_Shell* shell)
{
  if(fImg == 0) return 0;

  OS::ZGlassImg* list_img = 0;

  if(mSrcLinkName.length()) {
    if(mSrcLinkDatum == 0) {
      mSrcLinkDatum = fImg->GetLinkDatum(mSrcLinkName);
      if(mSrcLinkDatum == 0) {
	mSrcLinkName = "";
	return 0;
      }
    }
    list_img = mSrcLinkDatum->GetToImg();
  } else {
    list_img = fImg;
  }
  if(list_img == 0 && mSrcConfigPath.length()) {
    list_img = shell->SearchConfigEntry(mSrcConfigPath);
  }

  if(list_img && list_img->fIsList) return (ZList*)list_img->fGlass;
  return 0;
}
