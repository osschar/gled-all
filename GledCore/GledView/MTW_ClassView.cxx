// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// MTW_ClassView
//

#include "MTW_ClassView.h"

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
}

/**************************************************************************/
// MTW_ClassView
/**************************************************************************/

void MTW_ClassView::_init()
{  
  resizable(0);
  end();
  mWindow = dynamic_cast<Fl_Window*>(parent());

  // Setting for parent class.
  mFltkRep = this;
}

/**************************************************************************/

MTW_ClassView::MTW_ClassView(OS::ZGlassImg* img, FTW_Shell* shell) :
  MTW_View(img, shell),
  Fl_Pack(0,0,0,0)
{ _init(); }

MTW_ClassView::MTW_ClassView(ZGlass* glass, Fl_SWM_Manager* swm_mgr) :
  MTW_View(glass, swm_mgr),
  Fl_Pack(0,0,0,0)
{ _init(); }

MTW_ClassView::~MTW_ClassView()
{
  if(mWindow) {
    mWindow->remove(*this);
    delete mWindow;
  }
}

/**************************************************************************/


void MTW_ClassView::BuildVerticalView()
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
    //cout <<"MTW_ClassView::BuildVerticalView() ci="<< ci->fClassName <<endl;
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
  mSelfRep  = new SelfRep(this, 0, 0, mtw_vs.fUse.full, 1);
  add(mSelfRep);
  ++h;
  // Append one entry per c++ class; optionally insert class-header.
  for(lpMTW_SubView_i sv=mSubViews.begin(); sv!=mSubViews.end(); ++sv) {
    if(fancy_p) {
      add( new FGS::PackEntryCollapsor((*sv)->GetClassInfo()->fName.Data()));
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
    if(fImg->fIsList && mShell->GetShellInfo()->GetCollZList()) {
      if(children() > 5 ) { // AList
	FGS_DECLARE_CAST(pec, child(3), FGS::PackEntryCollapsor);
	if(pec) h += pec->collexp(false);
      }
      if(children() > 7 ) { // First descendant
	FGS_DECLARE_CAST(pec, child(5), FGS::PackEntryCollapsor);
	if(pec) h += pec->collexp(false);
      }
    }
  }

  size(mtw_vs.fUse.full, h);
  if(mWindow) mWindow->size(mtw_vs.fUse.full, h);
  auto_label();
}

void MTW_ClassView::BuildByLayout(MTW_Layout* layout)
{
  type(FL_HORIZONTAL);
  Fl_Group* ex_cur = Fl_Group::current();
  Fl_Group::current(this);
  int win_w = 0, win_h = 0;
  for(MTW_Layout::lClass_i c=layout->RefClasses().begin();
      c != layout->RefClasses().end(); ++c)
    {
      GNS::ClassInfo* cinfo = c->fClassInfo;
      if(GNS::IsA(mGlass, cinfo->fFid)) {
	MTW_SubView* sv = (cinfo->fViewPart->fooSVCreator)(cinfo, this, mGlass);
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

int MTW_ClassView::handle(int ev)
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

/**************************************************************************/
