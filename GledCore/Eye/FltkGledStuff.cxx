// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "FltkGledStuff.h"
#include "Eye.h"
#include "FTW_Shell.h"

#include "MCW_View.h"

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
  FTW_Shell_Client* c = grep_parent<FTW_Shell_Client*>(w);
  return c ? c->GetShell() : 0;
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
  draw_box();
  draw_label();
  fl_color(FL_BLACK);
  fl_font(labelfont(), labelsize());
  fl_push_clip(x()+3, y(), w()-6, h());
  fl_draw(mToName.c_str(), x()+3, y(), w()-6, h(), FL_ALIGN_LEFT, 0, 0);
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
	shell->FillImageMenu(fImg, menu, mcdl, "");

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
	shell->X_SetSource(id);
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
    ImagePasted(shell->GetEye()->DemangleID(source_id));
    return 1;
  }
  } // end switch(ev)

  return Fl_Box::handle(ev);
}

/**************************************************************************/
// LinkNameBox
/**************************************************************************/

FGS::LinkNameBox::LinkNameBox(OS::ZLinkDatum* ld, int x, int y, int w, int h, const char* t) :
  OS::ZLinkView(ld), LensNameBox(ld->GetToImg(),x,y,w,h,t)
{
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
