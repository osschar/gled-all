// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
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
#include <FL/Fl_Valuator.H>
#include <FL/Fl_Box.H>
#include <FL/Fl.H>
#include <FL/fl_draw.H>

namespace GNS  = GledNS;
namespace GVNS = GledViewNS;
namespace OS   = OptoStructs;
namespace FGS  = FltkGledStuff;

/**************************************************************************/
// Internal classes / data
/**************************************************************************/

namespace {

  float MaxAlignGrow = 1.2;
  float MaxJoinGrow  = 2;

  class View_SelfRep : public Fl_Box {
  public:
    MTW_View* fView;

    View_SelfRep(MTW_View* v, int w) :
      Fl_Box(0,0,w,1), fView(v)
    {
      box(FL_EMBOSSED_BOX);
      if(labelfont() < FL_BOLD)	labelfont(labelfont() + FL_BOLD);
      align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
      color(fl_rgb_color(220,200,200));
    }

    virtual int handle(int ev) {
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
	    ID_t id          = fView->fImg->fGlass->GetSaturnID();
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
	  if(shell->GetSource()->has_contents()) {
	    GNS::MethodInfo* cmi = fView->fImg->fClassInfo->FindMethodInfo("Add", true);
	    if(cmi) {
	      auto_ptr<ZMIR> mir (shell->GetSource()->generate_MIR(cmi, fView->fImg->fGlass));
	      fView->fImg->fEye->Send(*mir);
	    }
	  }
	  return 1;
	}
	} // switch (ev)
      } 

      return Fl_Box::handle(ev);
    }
  };

}

/**************************************************************************/
// MTW_View
/**************************************************************************/

void MTW_View::_init()
{
  bShown = false;
  resizable(0);
  end();
  mWindow = dynamic_cast<Fl_Window*>(parent());
}

void MTW_View::auto_label()
{
  if(mWindow) {
    m_label = mGlass ? 
      GForm("%s[%s]", mGlass->GetName(), mGlass->VGlassInfo()->fName.c_str()) :
      "<no-glass>";
    mWindow->label(m_label.c_str());

    View_SelfRep* vsr = dynamic_cast<View_SelfRep*>(child(0));
    if(vsr != 0) {
      vsr->copy_label(GForm("%s [ID=%d, RC=%d] '%s'",
			    mGlass->GetName(),
			    mGlass->GetSaturnID(), mGlass->GetRefCount(),
			    mGlass->GetTitle()));
    }
  }
}

/**************************************************************************/

MTW_View::MTW_View(OS::ZGlassImg* img, FTW_Shell* shell) :
  OS::A_View(img),
  FTW_SubShell(shell),
  Fl_Pack(0,0,0,0), Fl_SWM_Client(shell)
{
  mGlass = fImg->fGlass;
  _init();
}

MTW_View::MTW_View(ZGlass* glass, Fl_SWM_Manager* swm_mgr) :
  OS::A_View(0),
  FTW_SubShell(0),
  Fl_Pack(0,0,0,0), Fl_SWM_Client(swm_mgr)
{
  mGlass = glass;
  _init();
}

MTW_View::~MTW_View() {
  if(mWindow) {
    mWindow->remove(*this);
    delete mWindow;
  }
}

/**************************************************************************/

void MTW_View::BuildVerticalView()
{
  // Builds complete class-view for mGlass.
  // Weedgets are stacked vertically and resized to maximal width.

  int cell_w = swm_manager->cell_w();

  bool fancy_p      = mShell ? mShell->GetShellInfo()->GetFancyClassView() : false;
  bool show_links_p = mShell ? mShell->GetShellInfo()->GetShowLinksInClassView() : false;

  type(FL_VERTICAL);
  GNS::ClassInfo* ci = mGlass->VGlassInfo();
  MTW_Vertical_Stats mtw_vs;

  // Traverse class structure, create subviews.
  while(ci) {
    //cout <<"MTW_View::BuildVerticalView() ci="<< ci->fClassName <<endl;
    MTW_SubView* sv = (ci->fViewPart->fooSVCreator)(ci, this, mGlass);
    assert(sv);
    sv->BuildFromList(ci->fViewPart->fWeedList, show_links_p);
    sv->UpdateVerticalStats(mtw_vs, cell_w);
    mSubViews.push_front(sv);
    ci = ci->GetParentCI();
  }

  // mtw_vs.Dump();
  mtw_vs.Consolidate(MaxAlignGrow, MaxJoinGrow);
  // mtw_vs.Dump();

  // Start View construction.
  //-------------------------

  int h = 0;
  // Top matter.
  add( new View_SelfRep(this, mtw_vs.fUse.full) );
  ++h;
  // Append one entry per c++ class; optionally insert class-header.
  for(lpMTW_SubView_i sv=mSubViews.begin(); sv!=mSubViews.end(); ++sv) {
    if(fancy_p) {
      add( new FGS::PackEntryCollapsor((*sv)->mClassInfo->fName.c_str()));
      ++h;
    }
    h += (*sv)->ResizeByVerticalStats(mtw_vs, cell_w);
    add(*sv);
  }

  // Potentially collapse ZGlass/ZList SubViews.
  if(fancy_p) {
    if(mShell->GetShellInfo()->GetCollZGlass()) {
      FGS_DECLARE_CAST(pec, child(1), FGS::PackEntryCollapsor);
      if(pec) h += pec->collexp(false);
    }
    if(mShell->GetShellInfo()->GetCollZList() && children() > 5 ) {
      FGS_DECLARE_CAST(pec, child(3), FGS::PackEntryCollapsor);
      if(pec) h += pec->collexp(false);
    }
  }

  size(mtw_vs.fUse.full, h);
  if(mWindow) mWindow->size(mtw_vs.fUse.full, h);
  auto_label();
}

void MTW_View::BuildByLayout(MTW_Layout* layout)
{
  type(FL_HORIZONTAL);
  Fl_Group* ex_cur = Fl_Group::current();
  Fl_Group::current(this);
  int win_w = 0, win_h = 0;
  for(MTW_Layout::lClass_i c=layout->mClasses.begin(); c!=layout->mClasses.end(); ++c) {
    if(GNS::IsA(mGlass, c->fClassInfo->fFid)) {
      MTW_SubView* sv = (c->fClassInfo->fViewPart->fooSVCreator)(c->fClassInfo, this, mGlass);
      int x = 0, maxh=0;
      for(MTW_Layout::lMember_i m=c->fMembers.begin(); m!=c->fMembers.end(); ++m) {
	Fl_Widget* w = sv->CreateWeed(m->fWeedInfo);
	if(w == 0) {
	  w = new Fl_Box(FL_FLAT_BOX, x, 0, m->fW, m->fWeedInfo->fHeight, 0);
	} else {
	  w->resize(x, 0, m->fW, m->fWeedInfo->fHeight);
	  w->label(0);
	}
	x += m->fW;
	maxh = TMath::Max(maxh, m->fWeedInfo->fHeight);
      }
      sv->end();
      sv->resize(0,0,x,maxh);
      mSubViews.push_back(sv);
      win_h = TMath::Max(maxh, win_h);
    } else {
      Fl_Box* b = new Fl_Box(0,0,c->fFullW,1); b->box(FL_FLAT_BOX);
    }
    win_w += c->fFullW;
  }
  Fl_Group::current(ex_cur);
  size(win_w, win_h);
  if(mWindow) mWindow->size(win_w, win_h);
}

/**************************************************************************/

void MTW_View::AbsorbRay(Ray& ray)
{
  if(ray.IsBasicChange()) { auto_label(); redraw(); }

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
    if(update_all) {
      (*sv)->UpdateDataWeeds();
    } else {
      if((*sv)->mClassInfo->fFid == fid) {
	(*sv)->UpdateDataWeeds();
	break;
      }
    }
  }
}

void MTW_View::UpdateLinkWeeds(FID_t fid)
{
  bool update_all = fid.is_null();
  for(lpMTW_SubView_i sv=mSubViews.begin(); sv!=mSubViews.end(); ++sv) {
    if(update_all) {
      (*sv)->UpdateLinkWeeds();
    } else {
      if((*sv)->mClassInfo->fFid == fid) {
	(*sv)->UpdateLinkWeeds();
	break;
      }
    }
  }
}

/**************************************************************************/

void MTW_View::ShowWindow()
{
  if(mWindow) mWindow->show();
}

/**************************************************************************/

int MTW_View::handle(int ev)
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
  return Fl_Pack::handle(ev);
}
