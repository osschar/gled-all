// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// MTW_MetaView
//

#include "MTW_MetaView.h"

#include "MTW_SubView.h"
#include "FTW_Shell.h"

#include <Glasses/MetaViewInfo.h>
#include <Glasses/MetaSubViewInfo.h>
#include <Glasses/MetaWeedInfo.h>

#include <RegExp/pme.h>

#include <FL/Fl.H>

namespace GNS  = GledNS;
namespace GVNS = GledViewNS;
namespace OS   = OptoStructs;
namespace FGS  = FltkGledStuff;

/**************************************************************************/
// MTW_MetaView
/**************************************************************************/

void MTW_MetaView::_init()
{  
  resizable(0);
  end();
  mWindow = dynamic_cast<Fl_Window*>(parent());

  // Setting for parent class.
  mFltkRep = this;
}

/**************************************************************************/

MTW_MetaView::MTW_MetaView(OS::ZGlassImg* img, FTW_Shell* shell) :
  MTW_View(img, shell),
  Fl_Group(0,0,0,0)
{ _init(); }

MTW_MetaView::MTW_MetaView(ZGlass* glass, Fl_SWM_Manager* swm_mgr) :
  MTW_View(glass, swm_mgr),
  Fl_Group(0,0,0,0)
{ _init(); }

MTW_MetaView::~MTW_MetaView()
{
  if(mWindow) {
    mWindow->remove(*this);
    delete mWindow;
  }
}

/**************************************************************************/

namespace {
  void rebuild_cb(Fl_Widget* o, MTW_MetaView* ud) { ud->Rebuild(); }
}

void MTW_MetaView::BuildByLensGraph(ZGlass* gui)
{
  static const string _eh("MTW_MetaView::BuildByLensGraph ");

  MetaViewInfo* mvi = dynamic_cast<MetaViewInfo*>(gui);
  if(mvi == 0)
    throw(_eh + "top-level is not a MetaViewInfo.");

  Fl_Group* ex_cur = Fl_Group::current();
  Fl_Group::current(this);

  int W  = mvi->GetW(), H = mvi->GetH();
  int cX = 0, cY = 0; // precursor of some better layout management/packing

  if(mvi->GetExpertP()) {
    mGuiTemplate = mvi->GetSaturnID();

    mSelfRep = new SelfRep(this, 0, 0, W-8, 1);
    Fl_Button* b = new Fl_Button(W-8, 0, 8, 1, "Rebuild");
    b->callback((Fl_Callback*)rebuild_cb, this);
    
    H  += 1;
    cY += 1;
  }

  list<MetaSubViewInfo*> subviews;
  mvi->CopyByGlass<MetaSubViewInfo*>(subviews);
  list<MetaSubViewInfo*>::iterator subview_it = subviews.begin();
  while(subview_it != subviews.end()) {
    MetaSubViewInfo* msvi    = *subview_it;
    GNS::ClassInfo*  msvi_ci = GNS::FindClassInfo(msvi->GetName());
    
    if(msvi_ci == 0)
      throw(_eh + "glass '" + msvi->GetName() + "' not found.");
    if( ! GNS::IsA(mGlass, msvi_ci->fFid))
      throw(_eh + "source lens is not of glass '" + msvi->GetName() + "'.");

    // printf("  Sub: %s (%p)\n", msvi->GetName(), msvi_ci);

    MTW_SubView* sv = (msvi_ci->fViewPart->fooSVCreator)(msvi_ci, this, mGlass);

    list<MetaWeedInfo*> weeds;
    msvi->CopyByGlass<MetaWeedInfo*>(weeds);
    list<MetaWeedInfo*>::iterator weed_it = weeds.begin();
    while(weed_it != weeds.end()) {
      MetaWeedInfo*  mwi = *weed_it;

      string wname(mwi->GetName());
      PME       re("<(.*)>", "o");
      int rec = re.match(wname);

      Fl_Widget* o = 0;
      if(rec == 0) {
	GVNS::WeedInfo* wi = msvi_ci->fViewPart->FindWeedInfo(mwi->GetName());
	if(wi == 0)
	  throw(_eh + GForm("member '%s::%s' not found.", msvi->GetName(), mwi->GetName()));
      
	// printf("    W: %s (%p) %d,%d,%d,%d\n", mwi->GetName(), wi,
	//     mwi->GetX(), mwi->GetY(), mwi->GetW(), mwi->GetH());

	Fl_Widget* w = sv->CreateWeed(wi);
	w->resize(mwi->GetX() + cX, mwi->GetY() + cY, mwi->GetW(), mwi->GetH());
	o = w;
      } else {
	if(re[1] == "box") {
	  Fl_Box* b = new Fl_Box(mwi->GetX() + cX, mwi->GetY() + cY,
				 mwi->GetW(), mwi->GetH());
	  b->copy_label(mwi->GetTitle());
	  o = b;
	}
      }
      if(o == 0)
	throw(_eh + GForm("creation of '%s::%s' failed.", msvi->GetName(), mwi->GetName()));
      
      if(mwi->GetColorP()) {
	UChar_t c[3]; mwi->PtrColor()->rgb_to_ubyte(c);
	o->color(fl_rgb_color(c[0], c[1], c[2]));
      }
      if(mwi->GetLabelP()) {
	o->copy_label(mwi->GetTitle());
      }
      if(mwi->GetAlignP()) {
	int a = 0;
	if(mwi->GetAInside()) a |= FL_ALIGN_INSIDE;
	if(char l=mwi->GetALtRt()) a |= l<0 ? FL_ALIGN_LEFT : FL_ALIGN_RIGHT;
	if(char l=mwi->GetAUpDn()) a |= l<0 ? FL_ALIGN_TOP  : FL_ALIGN_BOTTOM;
	o->align(a);
      }
      if(mwi->GetBoxType() != 0) {
	Fl_Boxtype b;
	switch(mwi->GetBoxType()) {
	case MetaWeedInfo::BT_Flat:     b = FL_FLAT_BOX; break;
	case MetaWeedInfo::BT_Up:       b = FL_UP_BOX; break;
	case MetaWeedInfo::BT_ThinUp:   b = FL_THIN_UP_BOX; break;
	case MetaWeedInfo::BT_Engraved: b = FL_ENGRAVED_BOX; break;
	case MetaWeedInfo::BT_Border:   b = FL_BORDER_BOX; break;
	default: b = FL_FLAT_BOX;;
	}
	if(mwi->GetBDown())  b = fl_down(b);
	if(mwi->GetBFrame()) b = fl_frame(b);
	o->box(b);
      }

      ++weed_it;
    }

    sv->end();
    sv->size(W,H);
    add(sv);
    mSubViews.push_back(sv);

    ++subview_it;
  }

  Fl_Group::current(ex_cur);
  size(W, H);
  if(mWindow) mWindow->size(W,H);
  auto_label();
}

void MTW_MetaView::Rebuild()
{
  static const string _eh("MTW_MetaView::Rebuild ");

  OS::ZGlassImg* img = mShell->DemangleID(mGuiTemplate);
  if(img == 0) {
    mShell->Message(_eh + "GuiTemplate disappeared.", FTW_Shell::MT_err);
    return;
  }

  Fl_Window* w = mWindow ? new Fl_Window(mWindow->x(), mWindow->y()-10, 0, 0) :
                           new Fl_Window(0,0);
  MTW_MetaView* mv = new MTW_MetaView(fImg, mShell);
  w->end();
  try {
    mv->BuildByLensGraph(img->fGlass);
  }
  catch(string exc) {
    mShell->Message(exc, FTW_Shell::MT_err);
    delete w;
    return;
  }
  mShell->adopt_window(w);

  if(mWindow) {
    Fl::atclose(mWindow, 0);
  } else { 
    delete this;
  }
  w->show();
}

/**************************************************************************/

int MTW_MetaView::handle(int ev)
{
  // Suppress top-level dnd/paste.
  if(ev == FL_PASTE) return 1;

  if(ev == FL_SHOW) {
    bShown = true;
    UpdateDataWeeds(FID_t(0,0));
    UpdateLinkWeeds(FID_t(0,0));
  } else if(ev == FL_HIDE) {
    bShown = false;
  }
  return Fl_Group::handle(ev);
}

/**************************************************************************/
