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
#include <FL/fl_draw.H>

namespace GNS  = GledNS;
namespace GVNS = GledViewNS;
namespace OS   = OptoStructs;

MTW_View::MTW_View(OS::ZGlassImg* img) :
  OS::A_GlassView(img), Fl_Pack(0,0,0,0,0)
{
  // end() called from BuildVerticalView() or BuildByLayout()

  mGlass = fImg->fGlass;
  bShown = false;
  resizable(0);
}

MTW_View::MTW_View(ZGlass* glass) :
  OS::A_GlassView(0), Fl_Pack(0,0,0,0,0)
{
  // end() called from BuildVerticalView() or BuildByLayout()

  mGlass = glass;
  resizable(0);
}

MTW_View::~MTW_View() {
  // fltk deletes all
}

/**************************************************************************/

static float MaxAlignGrow = 1.1;
static float MaxJoinGrow  = 1.4;

void MTW_View::BuildVerticalView(int cell_w)
{
  // Builds complete view of a given object
  // Weedgets are stacked vertically and resized to maximal width
  // !! This should be exception throwing
  // !! And GledViewNS creators even more so ...
  
  type(FL_VERTICAL);
  GNS::ClassInfo* ci = mGlass->VGlassInfo();
  MTW_Vertical_Stats mtw_vs;

  Fl_Group::current(0); // Must reverse order of insertion ...
  while(ci) {
    //cout <<"MTW_View::BuildVerticalView() ci="<< ci->fClassName <<endl;
    MTW_SubView* sv = (ci->fViewPart->fooSVCreator)(ci, this, mGlass);
    assert(sv);
    sv->BuildFromList(ci->fViewPart->fWeedList);
    sv->UpdateVerticalStats(mtw_vs, cell_w);
    mSubViews.push_front(sv);
    ci = ci->GetParentCI();
  } while(ci);

  // mtw_vs.Dump();
  mtw_vs.Consolidate(MaxAlignGrow, MaxJoinGrow);
  // mtw_vs.Dump();
  int h = 0;
  for(lpMTW_SubView_i sv=mSubViews.begin(); sv!=mSubViews.end(); ++sv) {
    h += (*sv)->ResizeByVerticalStats(mtw_vs, cell_w);
    add(*sv);
  }
  Fl_Group::current(this);
  end();

  // cout <<"ended with h=" << h << endl;
  size(mtw_vs.fUse.full, h);
}

void MTW_View::BuildByLayout(MTW_Layout* layout)
{
  type(FL_HORIZONTAL);
  for(MTW_Layout::lClass_i c=layout->mClasses.begin(); c!=layout->mClasses.end(); ++c) {
    if(GNS::IsA(mGlass, c->fClassInfo->fFid)) {
      MTW_SubView* sv = (c->fClassInfo->fViewPart->fooSVCreator)(c->fClassInfo, this, mGlass);
      int x = 0, maxh=0;
      for(MTW_Layout::lMember_i m=c->fMembers.begin(); m!=c->fMembers.end(); ++m) {
	Fl_Widget* w = (m->fWeedInfo->fooWCreator)(sv);
	w->resize(x, 0, m->fW, m->fWeedInfo->fHeight);
	w->label(0);
	x += m->fW;
	maxh = TMath::Max(maxh, m->fWeedInfo->fHeight);
	sv->mWeeds.push_back( MTW_Weed(w, m->fWeedInfo) );
      }
      sv->end();
      sv->resize(0,0,x,maxh);
      mSubViews.push_back(sv);
    } else {
      Fl_Box* b = new Fl_Box(0,0,c->fFullW,1); b->box(FL_FLAT_BOX);
    }
  }
  end();
}

/**************************************************************************/

/*
void MTW_View::Retitle()
{
  ostrstream title;
  title << mNode->GetName() << "[" << mNode->ClassName() << "]";
  label(title.str());
}
*/

void MTW_View::AbsorbRay(Ray& ray)
{
  if(bShown && ray.fRQN == RayNS::RQN_change) {
    UpdateViews(ray.fFID);
  }
}

/**************************************************************************/

void MTW_View::UpdateViews(FID_t fid)
{
  bool update_all = fid.is_null();
  for(lpMTW_SubView_i sv=mSubViews.begin(); sv!=mSubViews.end(); ++sv) {
    if(update_all) {
      (*sv)->Update();
    } else {
      if((*sv)->mClassInfo->fFid == fid) {
	(*sv)->Update();
	break;
      }
    }
  }
}

/**************************************************************************/
/**************************************************************************/

namespace {

  class MTW_View_Window : public Fl_Window, public Fl_SWM_Client,
			  public OS::A_GlassView
  {
  protected:
    string m_title_label;
  public:
    MTW_View_Window(OS::ZGlassImg* img, int x, int y, const char* t=0) :
      Fl_Window(x,y,t), A_GlassView(img) {}

    ~MTW_View_Window() {}

    void auto_label() { 
      if(fImg) {
	m_title_label = GForm("%s[%s]", fImg->fGlass->GetName(),
			      fImg->fClassInfo->fName.c_str());
	label(m_title_label.c_str());
      }
    }

    virtual void AssertDependantViews() {}
    virtual void AbsorbRay(Ray& ray) {
      if(ray.IsBasicChange()) { auto_label(); redraw(); }
    }
  };

}

Fl_Window* MTW_View::ConstructVerticalWindow(OS::ZGlassImg* img,
					     Fl_SWM_Manager* swm_mgr)
{
  // Some controls &| collapsors would be usefull.

  MTW_View_Window* w = new MTW_View_Window(img, 0, 0, 0);
  w->auto_label();

  MTW_View* v = new MTW_View(img);
  int cell_w = 0;
  if(swm_mgr) {
    fl_font(fl_font(), swm_mgr->cell_fontsize());
    cell_w = swm_mgr->cell_w();
  }
  v->BuildVerticalView(cell_w);
  w->end();
  w->size(v->w(), v->h());
  return w;
}

Fl_Window* MTW_View::ConstructVerticalWindow(ZGlass* glass,
					     Fl_SWM_Manager* swm_mgr)
{
  Fl_Window* w = new MTW_View_Window(0, 0, 0, glass->GetName());
  MTW_View* v = new MTW_View(glass);
  int cell_w = 0;
  if(swm_mgr) {
    fl_font(fl_font(), swm_mgr->cell_fontsize());
    cell_w = swm_mgr->cell_w();
  }
  v->BuildVerticalView(cell_w);
  w->end();
  w->size(v->w(), v->h());
  return w;
}

/**************************************************************************/

int MTW_View::handle(int ev)
{
  if(ev == FL_SHOW) {
    bShown = true;
    UpdateViews(FID_t(0,0));
  } else if(ev == FL_HIDE) {
    bShown = false;
  }
  return Fl_Pack::handle(ev);
}
