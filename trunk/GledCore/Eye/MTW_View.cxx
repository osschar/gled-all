// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
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

namespace GV = GledViewNS;
namespace OS = OptoStructs;

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
static float MaxJoinGrow  = 1.2;

void MTW_View::BuildVerticalView()
{
  // Builds complete view of a given object
  // Weedgets are stacked vertically and resized to maximal width
  // !! This should be exception throwing
  // !! And GledViewNS creators even more so ...
  
  type(FL_VERTICAL);
  GV::ClassInfo* ci = GV::FindClassInfo(mGlass->ZID());
  MTW_Vertical_Stats mtw_vs;

  Fl_Group::current(0); // Must reverse order of insertion ...
  while(ci) {
    //cout <<"MTW_View::BuildVerticalView() ci="<< ci->fClassName <<endl;
    MTW_SubView* sv = (ci->fooSVCreator)(ci, this, mGlass);
    assert(sv);
    sv->BuildFromList(ci->fMIlist);
    sv->UpdateVerticalStats(mtw_vs);
    mSubViews.push_front(sv);
    ci = ci->GetParentCI();
  } while(ci);

  // mtw_vs.Dump();
  mtw_vs.Consolidate(MaxAlignGrow, MaxJoinGrow);
  // mtw_vs.Dump();
  int h = 0;
  for(lpMTW_SubView_i sv=mSubViews.begin(); sv!=mSubViews.end(); ++sv) {
    h += (*sv)->ResizeByVerticalStats(mtw_vs);
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
    if(GledNS::IsA(mGlass, c->fClassInfo->fFid)) {
      MTW_SubView* sv = (c->fClassInfo->fooSVCreator)(c->fClassInfo, this, mGlass);
      int x = 0, maxh=0;
      for(MTW_Layout::lMember_i m=c->fMembers.begin(); m!=c->fMembers.end(); ++m) {
	Fl_Widget* w = (m->fMemberInfo->fooWCreator)(sv);
	w->resize(x, 0, m->fW, m->fMemberInfo->fHeight);
	w->label(0);
	x += m->fW;
	maxh = maxh >? m->fMemberInfo->fHeight;
	sv->mWeeds.push_back( MTW_Weed(w, m->fMemberInfo) );
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

void MTW_View::Absorb_Change(LID_t lid, CID_t cid)
{
  if(bShown) {
    FID_t fid(lid,cid);
    bool update_all = (lid==0 && cid==0);
    fImg->fGlass->RefExecMutex().Lock();
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
    fImg->fGlass->RefExecMutex().Unlock();
  }
}

/**************************************************************************/

void MTW_View::UpdateViews(LID_t lid, CID_t cid)
{
  Absorb_Change(lid, cid);
}

/**************************************************************************/
/**************************************************************************/

Fl_Window* MTW_View::ConstructVerticalWindow(OS::ZGlassImg* img)
{
  // Akhem ... should be some fl_window subclass, knowing of
  // mtw_view so that it can be wiped.
  // Also some controls &| collapsors would be usefull.

  Fl_Window* w = new Fl_Window(0,0,GForm("%s[%s]", img->fGlass->GetName(),
					img->fClassInfo->fClassName.c_str()));
  MTW_View* v = new MTW_View(img);
  v->BuildVerticalView();
  w->end();
  w->size(v->w(), v->h());
  return w;
}

Fl_Window* MTW_View::ConstructVerticalWindow(ZGlass* glass)
{
  Fl_Window* w = new Fl_Window(0,0,glass->GetName());
  MTW_View* v = new MTW_View(glass);
  v->BuildVerticalView();
  w->end();
  w->size(v->w(), v->h());
  return w;
}

/**************************************************************************/

int MTW_View::handle(int ev)
{
  if(ev == FL_SHOW) {
    bShown = true;
    Absorb_Change(0, 0);
  } else if(ev == FL_HIDE) {
    bShown = false;
  }
  return Fl_Pack::handle(ev);
}
