// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.


#include "MTW_View.h"
#include "MTW_SubView.h"
#include "MTW_Layout.h"
#include "FTW_Shell.h"
#include <Glasses/ZGlass.h>
#include <Glasses/ShellInfo.h>
#include <Stones/ZMIR.h>
#include <Eye/Eye.h>
#include <Gled/GledNS.h>

#include <GledView/GledViewNS.h>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl.H>
#include <FL/fl_draw.H>

namespace GNS  = GledNS;
namespace GVNS = GledViewNS;
namespace OS   = OptoStructs;
namespace FGS  = FltkGledStuff;


/**************************************************************************/
// MTW_View
/**************************************************************************/

void MTW_View::_init()
{
  mFltkRep = 0; // Fltk representation.
  bShown   = false;
  mSelfRep = 0;
}

void MTW_View::auto_label()
{
  if(mWindow && mWindow->parent() == 0) {
    m_window_label = mGlass ?
      GForm("%s[%s]", mGlass->GetName(), mGlass->VGlassInfo()->fName.Data()) :
      "<no-glass>";
    mWindow->label(m_window_label.Data());
  }
  if(mSelfRep) {
    mSelfRep->copy_label(GForm("%s [ID=%d, RC=%d] '%s'",
			       mGlass->GetName(),
			       mGlass->GetSaturnID(), mGlass->GetRefCount(),
			       mGlass->GetTitle()));
  }
}

/**************************************************************************/

MTW_View::MTW_View(OS::ZGlassImg* img, FTW_Shell* shell) :
  OS::A_View(img),
  FTW_SubShell(shell),
  Fl_SWM_Client(shell)
{
  mGlass = fImg->fLens;
  _init();
}

MTW_View::MTW_View(ZGlass* glass, Fl_SWM_Manager* swm_mgr) :
  OS::A_View(0),
  FTW_SubShell(0),
  Fl_SWM_Client(swm_mgr)
{
  mGlass = glass;
  _init();
}

MTW_View::~MTW_View()
{}

/**************************************************************************/

void MTW_View::Labelofy()
{
  // Sets labels to sub-views ... resizes self.

  int dww = 0;
  for(lpMTW_SubView_i sv=mSubViews.begin(); sv!=mSubViews.end(); ++sv) {
    int dx = 0;
    for(lMTW_Weed_i w=(*sv)->RefWeeds().begin(); w!=(*sv)->RefWeeds().end(); ++w) {
      Fl_Widget*       o = w->fWeed;
      GVNS::WeedInfo& wi = *w->fWeedInfo;
      o->label(w->fWeedInfo->fName.Data());
      if(wi.bLabel && !wi.bLabelInside) {
	o->align(FL_ALIGN_LEFT);
	dx += FGS::swm_label_width(w->fWeedInfo->fName, swm_manager->cell_w());
	o->position(o->x()+dx, o->y());
      }
    }
    (*sv)->size((*sv)->w() + dx, (*sv)->y());
    dww += dx;
  }
  mFltkRep->size(mFltkRep->w() + dww, mFltkRep->h());
}

/**************************************************************************/

void MTW_View::AbsorbRay(Ray& ray)
{
  if(ray.IsBasicChange()) { auto_label(); }

  if(bShown) {
    if(ray.fRQN == RayNS::RQN_change) {
      UpdateDataWeeds(ray.fFID);
    }
    else if(ray.fRQN == RayNS::RQN_link_change) {
      UpdateLinkWeeds(ray.fFID);
    }
  }
}

/**************************************************************************/

void MTW_View::UpdateDataWeeds(FID_t fid)
{
  bool update_all = fid.is_null();
  for(lpMTW_SubView_i sv=mSubViews.begin(); sv!=mSubViews.end(); ++sv) {
    if(update_all || (*sv)->GetClassInfo()->fFid == fid)
      (*sv)->UpdateDataWeeds();
  }
}

void MTW_View::UpdateLinkWeeds(FID_t fid)
{
  bool update_all = fid.is_null();
  for(lpMTW_SubView_i sv=mSubViews.begin(); sv!=mSubViews.end(); ++sv) {
    if(update_all || (*sv)->GetClassInfo()->fFid == fid)
      (*sv)->UpdateLinkWeeds();
  }
}

/**************************************************************************/
// SelfRep
/**************************************************************************/

MTW_View::SelfRep::SelfRep(MTW_View* v, int x, int y, int w, int h) :
      Fl_Box(x,y,w,h), fView(v)
{
  box(FL_EMBOSSED_BOX);
  if(labelfont() < FL_BOLD)	labelfont(labelfont() + FL_BOLD);
  align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  color(fl_rgb_color(220,200,200));
}

int MTW_View::SelfRep::handle(int ev) {
  FTW_Shell* shell = fView->GetShell();
  if(shell != 0) {
    switch (ev) {

    case FL_PUSH: {
      if(Fl::event_button() == 2) {
	Fl::paste(*this);
      }
      if(Fl::event_button() == 3) {
	shell->FullMenu(fView->fImg, Fl::event_x_root(), Fl::event_y_root());
      }
      return 1;
    }

    case FL_DRAG: {
      if(Fl::event_state(FL_BUTTON1) && ! Fl::event_inside(this)) {
	ID_t id          = fView->fImg->fLens->GetSaturnID();
	const char* text = GForm("%u", id);
	shell->X_SetSource(fView->fImg);
	Fl::copy(text, strlen(text), 0);
	Fl::dnd();
	return 1;
      }
    }

    case FL_DND_ENTER:
      return (fView->fImg->fIsList) ? 1 : 0;

    case FL_DND_LEAVE:
      return 1;

    case FL_DND_RELEASE:
      return (Fl::belowmouse() == this) ? 1 : 0;

    case FL_PASTE: {
      if(fView->fImg->fIsList && shell->GetSource()->has_contents()) {
	AList* l = (AList*) fView->fImg->fLens;
	auto_ptr<ZMIR> mir ( l->MkMir_Add(0) );
	shell->GetSource()->fix_MIR_beta(mir);
	fView->fImg->fEye->Send(*mir);
      }
      return 1;
    }
    } // switch (ev)
  }

  return Fl_Box::handle(ev);
}
