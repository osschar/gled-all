// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.


#include "MTW_View.h"
#include "MTW_SubView.h"
#include "MTW_Layout.h"
#include <Glasses/ZGlass.h>
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

/**************************************************************************/
// Internal classes / data
/**************************************************************************/

namespace {

  float MaxAlignGrow = 1.2;
  float MaxJoinGrow  = 2;

  class SubView_Header;
  void cb_coll(Fl_Button* w, SubView_Header* svh);

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
	if(ev == FL_PUSH) {
	  if(Fl::event_button() == 1) {
	    return 1;
	  }
	  if(Fl::event_button() == 3) {
	    shell->ImageMenu(fView->fImg, Fl::event_x_root(), Fl::event_y_root());
	  }
	}
	if(ev == FL_DRAG && Fl::event_state(FL_BUTTON1) &&
	   ! Fl::event_inside(this))
	  {
	    ID_t id          = fView->fImg->fGlass->GetSaturnID();
	    const char* text = GForm("%u", id);
	    shell->X_SetSource(id);
	    Fl::copy(text, strlen(text), 0);
	    Fl::dnd();
	    return 1;
	  }
      }

      return Fl_Box::handle(ev);
    }
  };

  class SubView_Header : public Fl_Group {
  public:
    Fl_Button*   fBut;
    MTW_SubView* fSubView;

    SubView_Header(MTW_SubView* sv, int w, const char* t) :
      Fl_Group(0,0,w,1), fSubView(sv)
    {
      fBut = new Fl_Button(0,0,2,1, "@#>");
      fBut->labeltype(FL_SYMBOL_LABEL);
      fBut->callback((Fl_Callback*)cb_coll, this);
      fBut->color(fl_rgb_color(200,220,200));

      Fl_Box* b = new Fl_Box(2,0,w-2,1, t);
      b->box(FL_EMBOSSED_BOX);
      if(b->labelfont() < FL_BOLD) b->labelfont(b->labelfont() + FL_BOLD);
      b->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
      b->color(fl_rgb_color(200,220,200));
    }

    void collapse() { fSubView->hide(); fBut->label("@#>[]"); }
    void expand()   { fSubView->show(); fBut->label("@#>"); }
  };

  void cb_coll(Fl_Button* w, SubView_Header* svh) {
    if(svh->fSubView->visible()) {
      svh->collapse();
      svh->fSubView->GetView()->ReHeight(-svh->fSubView->h());
    } else {
      svh->expand();
      svh->fSubView->GetView()->ReHeight(svh->fSubView->h());
    }
  }
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
      vsr->copy_label(GForm("[ID=%d, RC=%d] '%s' '%s'",
			    mGlass->GetSaturnID(), mGlass->GetRefCount(),
			    mGlass->GetName(), mGlass->GetTitle()));
    }
  }
}

/**************************************************************************/

MTW_View::MTW_View(OS::ZGlassImg* img, FTW_Shell* shell) :
  OS::A_View(img),
  FTW_Shell_Client(shell),
  Fl_Pack(0,0,0,0), Fl_SWM_Client(shell)
{
  mGlass = fImg->fGlass;
  _init();
}

MTW_View::MTW_View(ZGlass* glass, Fl_SWM_Manager* swm_mgr) :
  OS::A_View(0),
  FTW_Shell_Client(0),
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
      add( new SubView_Header(*sv, mtw_vs.fUse.full,
			      (*sv)->mClassInfo->fName.c_str()) );
      ++h;
    }
    h += (*sv)->ResizeByVerticalStats(mtw_vs, cell_w);
    add(*sv);
  }

  // Potentially collapse ZGlass/ZList SubViews.
  if(fancy_p) {
    if(mShell->GetShellInfo()->GetCollZGlass()) {
      SubView_Header *svh = dynamic_cast<SubView_Header*>(child(1));
      if(svh) {
	svh->collapse();
	h -= svh->fSubView->h();
      }
    }
    if(mShell->GetShellInfo()->GetCollZList() && children() > 5 ) {
      SubView_Header *svh = dynamic_cast<SubView_Header*>(child(3));
      if(svh && GNS::IsA(mGlass, ZList::FID())) {
	svh->collapse();
	h -= svh->fSubView->h();
      }
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

void MTW_View::ReHeight(int dh)
{
  // Called from callbacks.

  init_sizes();
  redraw();
  if(mWindow) {
    mWindow->position(mWindow->x(), mWindow->y() + dh/2);
    mWindow->size(w(), h()+dh);
    mWindow->redraw();
  }
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
