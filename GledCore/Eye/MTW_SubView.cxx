// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "MTW_SubView.h"
#include <FL/Fl_Widget.H>

#include <math.h>

MTW_SubView::MTW_SubView(GledViewNS::ClassInfo* ci, MTW_View* v) :
  Fl_RelGroup(0,0,0,0,0), mClassInfo(ci), mView(v)
{
  // end() called from BuildFromList()

  resizable(0);
}

MTW_SubView::~MTW_SubView() {
  // weeds deleted by Fl_Group
}

/**************************************************************************/

void MTW_SubView::Update()
{
  for(lMTW_Weed_i i=mWeeds.begin(); i!=mWeeds.end(); ++i) {
    (i->fMemberInfo->fooWUpdate)(i->fWeed, this);
  }
}

/**************************************************************************/

void MTW_SubView::BuildFromList(GledViewNS::lpMemberInfo_t& l)
{
  for(GledViewNS::lpMemberInfo_i i=l.begin(); i!=l.end(); ++i) {
    Fl_Widget* w = ((*i)->fooWCreator)(this);
    mWeeds.push_back( MTW_Weed(w, *i) );
    // cout <<" MTW_SubView::BuildFromList() adding member "<< (*i)->fName << endl; 
  }
  end();
}

/**************************************************************************/

void MTW_SubView::UpdateVerticalStats(MTW_Vertical_Stats& vs)
{
  bool in_join = false;
  int  join_w  = 0;
  for(lMTW_Weed_i i=mWeeds.begin(); i!=mWeeds.end(); ++i) {
    GledViewNS::MemberInfo& mi = *i->fMemberInfo;
    int full_w = 0, lab_w = 0, weed_w = mi.fWidth;
    if(mi.bLabel) {
      if(mi.bLabelInside) {
	weed_w += mi.fName.size();
      } else {
	lab_w = mi.fName.size();
	vs.fMaxOutsideLabeledW = vs.fMaxOutsideLabeledW >? weed_w;
	vs.fMaxLabelW          = vs.fMaxLabelW          >? lab_w;
      }
    }
    full_w = lab_w + weed_w;
    vs.fMaxFullW = vs.fMaxFullW >? full_w;

    if(in_join) {
      join_w += full_w;
      lMTW_Weed_i ii = i; ++ii;
      if(! mi.bJoinNext || ii==mWeeds.end()) {
	vs.fMaxGreedyJoinW = vs.fMaxGreedyJoinW >? join_w;
	in_join = false;
      }
    } else {
      if(mi.bJoinNext) { join_w = full_w; in_join = true; }
    }
  }
}

int MTW_SubView::ResizeByVerticalStats(MTW_Vertical_Stats& vs)
{
  int y = 0; // current pos
  MTW_Widths& lim = vs.fUse;
  for(lMTW_Weed_i i=mWeeds.begin(); i!=mWeeds.end(); ) {
    MTW_Widths w = i->GetWidths();
    // Harvest the joiners
    lMTW_Weed_i j = i; ++j;
    int n_join = 1;
    int max_h  = i->fMemberInfo->fHeight;;
    {
      lMTW_Weed_i ii = i; 
      while(j != mWeeds.end() && ii->fMemberInfo->bJoinNext) {
	MTW_Widths z = j->GetWidths();
	if(z.full + w.full > lim.full) break;
	max_h = max_h >? j->fMemberInfo->fHeight;
	w += z; ii = j++; ++n_join;
      }
    }
    
    if(n_join > 1) { // Set multiple weeds
      int  taken = 0;
      int  align = FL_ALIGN_LEFT;
      bool flip_align = (n_join==2 && vs.bRightLabelPair);
      while(i != j) {
	GledViewNS::MemberInfo& mi = *i->fMemberInfo;
	int ideal = int( roundf(float(lim.full - taken)/n_join) );
	MTW_Widths z = i->GetWidths();
	int dw = ideal - z.full;
	if(dw > 0) {
	  if(mi.bLabel && !mi.bLabelInside) {
	    int dl = mi.bCanResize ? int( roundf(0.25*dw) ) : dw;
	    z.label += dl; dw -= dl;
	  }
	  if(mi.bCanResize) z.weed += dw;
	  z.full = z.label + z.weed;
	}
	int h = mi.bCanResize ? max_h : mi.fHeight;

	if(flip_align && n_join==1) {
	  i->fWeed->resize(taken, y, z.weed, h);
	} else {
	  i->fWeed->resize(taken + z.label, y, z.weed, h);
	}
	if(mi.bLabel && !mi.bLabelInside) i->fWeed->align(align);

	taken += z.full;
	++i; --n_join;
	if(flip_align) align = FL_ALIGN_RIGHT;
      }
    } else {	  // Set a single weed
      GledViewNS::MemberInfo& mi = *i->fMemberInfo;
      int free = lim.full - w.full;
      if(mi.bLabel && !mi.bLabelInside && w.label<lim.label) {
	int dl = lim.label - w.label;
	if(free <= dl) {
	  w.label += free; free = 0;
	} else {
	  w.label += dl; free -= dl;
	}
      }
      if(free > 0 && mi.bCanResize) {
	w.weed += free;
      }
      i->fWeed->resize(w.label, y, w.weed, max_h);
      if(mi.bLabel && !mi.bLabelInside) i->fWeed->align(FL_ALIGN_LEFT);
    }
    y += max_h;
    i = j;
  } // for weeds
  size(lim.full, y);
  return y;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
// MTW_Weed
/**************************************************************************/

MTW_Widths MTW_Weed::GetWidths()
{
  MTW_Widths ret;
  ret.weed = fMemberInfo->fWidth;
  if(fMemberInfo->bLabel) {
    if(fMemberInfo->bLabelInside) {
      ret.weed += fMemberInfo->fName.size();
      ret.label = 0;
    } else {
      ret.label = fMemberInfo->fName.size();
    }
  }
  ret.full = ret.weed + ret.label;
  return ret;
}

/**************************************************************************/
// MTW_Vertical_Stats
/**************************************************************************/

MTW_Vertical_Stats::MTW_Vertical_Stats() : fUse() {
  fMaxFullW = fMaxGreedyJoinW = 0;
  fMaxOutsideLabeledW = fMaxLabelW = 0;
  bRightLabelPair = true;
}

void MTW_Vertical_Stats::Consolidate(float max_align_grow, float max_join_grow)
{
  float align_fac = max_align_grow <?
    float(fMaxLabelW + fMaxOutsideLabeledW)/fMaxFullW;
  float join_fac  = max_join_grow <?
    float(fMaxGreedyJoinW)/fMaxFullW;

  if(align_fac>1 || join_fac>1) {
    float grow_fac = align_fac >? join_fac;
    fUse.full = int(ceilf(grow_fac * fMaxFullW));
  } else {
    fUse.full = fMaxFullW;
  }
  if(fUse.full % 2 != 0) ++fUse.full;
    
  if(fUse.full < fMaxLabelW + fMaxOutsideLabeledW) {
    fUse.label = fUse.full - fMaxOutsideLabeledW;
    fUse.weed  = fMaxOutsideLabeledW;
  } else {
    fUse.label = fMaxLabelW;
    fUse.weed  = fUse.full - fMaxLabelW;
  }
}

void MTW_Vertical_Stats::Dump() {
  cout <<"fMaxFullW="<< fMaxFullW <<", "
       <<"fMaxGreedyJoinW="<< fMaxGreedyJoinW <<", "
       <<"fMaxOutsideLabeledW="<< fMaxOutsideLabeledW <<",\n"
       <<"fMaxLabelW="<< fMaxLabelW <<", "
       <<"bRightLabelPair="<< bRightLabelPair <<", "
       <<"Use("<< fUse.full <<","<< fUse.weed <<","<< fUse.label <<")\n";
    
}

/**************************************************************************/
// MTW_Widths
/**************************************************************************/

MTW_Widths& MTW_Widths::operator+=(MTW_Widths& r)
{
  full += r.full; weed += r.weed; label += r.label;
  return *this;
}
