// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "MTW_SubView.h"
#include "MTW_View.h"
#include "FltkGledStuff.h"

#include <Glasses/ZGlass.h>

#include <FL/Fl_Widget.H>
#include <FL/Fl.H>
#include <FL/fl_draw.H>

#include <math.h>

namespace GNS  = GledNS;
namespace GVNS = GledViewNS;
namespace FGS  = FltkGledStuff;
namespace OS   = OptoStructs;

/**************************************************************************/

const int     MTW_SubView::MinLabelWidth  = 3;
const Text_t* MTW_SubView::sTextorTooltip = "press Enter to send data";

/**************************************************************************/

MTW_SubView::MTW_SubView(GNS::ClassInfo* ci, MTW_View* v) :
  Fl_Group(0,0,0,0,0), mClassInfo(ci), mView(v)
{
  // end() called from BuildFromList()

  resizable(0);
}

MTW_SubView::~MTW_SubView()
{
  // weeds deleted by Fl_Group
  RemoveUpdateTimer();
}

/**************************************************************************/

Fl_Widget* MTW_SubView::CreateWeed(GVNS::WeedInfo* wi)
{
  Fl_Widget* w = (wi->fooWCreator)(this);
  if(w == 0) return 0;
  if(wi->bIsLinkWeed) {
    mLinkWeeds.push_back( MTW_Weed(w, wi) );
  } else {
    mDataWeeds.push_back( MTW_Weed(w, wi) );
  }
  mWeeds.push_back( MTW_Weed(w, wi) );
  return w;
}

/**************************************************************************/

void MTW_SubView::Update()
{
  UpdateDataWeeds();
  UpdateLinkWeeds();
}

void MTW_SubView::UpdateDataWeeds()
{
  RemoveUpdateTimer();
  for(lMTW_Weed_i i=mDataWeeds.begin(); i!=mDataWeeds.end(); ++i) {
    (i->fWeedInfo->fooWUpdate)(i->fWeed, this);
  }
}

void MTW_SubView::UpdateLinkWeeds()
{
  for(lMTW_Weed_i i=mLinkWeeds.begin(); i!=mLinkWeeds.end(); ++i) {
    (i->fWeedInfo->fooWUpdate)(i->fWeed, this);
  }
}

/**************************************************************************/

void MTW_SubView::SetUpdateTimer()
{
  // cout << "Setting update timer\n";
  Fl::remove_timeout((Fl_Timeout_Handler)UpdateFromTimer_s, this);
  Fl::add_timeout(1, (Fl_Timeout_Handler)UpdateFromTimer_s, this);
}

void MTW_SubView::RemoveUpdateTimer()
{
  // cout << "Removing update timer\n";
  Fl::remove_timeout((Fl_Timeout_Handler)UpdateFromTimer_s, this);
}

// static
void MTW_SubView::UpdateFromTimer_s(MTW_SubView* v)
{
  // cout << "Timer update !!!\n";
  v->mView->fImg->fLens->ReadLock();
  v->UpdateDataWeeds();
  v->mView->fImg->fLens->ReadUnlock();
}

/**************************************************************************/

void MTW_SubView::BuildFromList(GVNS::lpWeedInfo_t& l, bool show_links)
{
  for(GVNS::lpWeedInfo_i i=l.begin(); i!=l.end(); ++i) {
    if((*i)->bIsLinkWeed && show_links == false)
      continue;
    CreateWeed(*i);
    // cout <<" MTW_SubView::BuildFromList() adding member "<< (*i)->fName << endl;
  }
  end();
}

/**************************************************************************/

void MTW_SubView::UpdateVerticalStats(MTW_Vertical_Stats& vs, int cell_w)
{
  bool in_join = false;
  int  join_w  = 0;
  for(lMTW_Weed_i i=mWeeds.begin(); i!=mWeeds.end(); ++i) {
    GVNS::WeedInfo& mi = *i->fWeedInfo;
    int full_w = 0, lab_w = 0, weed_w = mi.fWidth;
    if(mi.bLabel) {
      if(mi.bLabelInside) {
	weed_w += FGS::swm_string_width(i->fWeedInfo->fName, cell_w);
      } else {
	lab_w = TMath::Max(FGS::swm_label_width(i->fWeedInfo->fName, cell_w),
			   MinLabelWidth);
	vs.fMaxOutsideLabeledW = TMath::Max(vs.fMaxOutsideLabeledW, weed_w);
	vs.fMaxLabelW          = TMath::Max(vs.fMaxLabelW, lab_w);
      }
    }
    full_w = lab_w + weed_w;
    vs.fMaxFullW = TMath::Max(vs.fMaxFullW, full_w);

    if(in_join) {
      join_w += full_w;
      lMTW_Weed_i ii = i; ++ii;
      if(! mi.bJoinNext || ii==mWeeds.end()) {
	vs.fMaxGreedyJoinW = TMath::Max(vs.fMaxGreedyJoinW, join_w);
	in_join = false;
      }
    } else {
      if(mi.bJoinNext) { join_w = full_w; in_join = true; }
    }
  }
}

int MTW_SubView::ResizeByVerticalStats(MTW_Vertical_Stats& vs, int cell_w)
{
  int y = 0; // current pos
  MTW_Widths& lim = vs.fUse;
  for(lMTW_Weed_i i=mWeeds.begin(); i!=mWeeds.end(); ) {
    MTW_Widths w = i->GetWidths(cell_w);
    // Harvest the joiners
    lMTW_Weed_i j = i; ++j;
    int n_join = 1;
    int max_h  = i->fWeedInfo->fHeight;
    {
      lMTW_Weed_i ii = i;
      while(j != mWeeds.end() && ii->fWeedInfo->bJoinNext) {
	MTW_Widths z = j->GetWidths(cell_w);
	if(z.full + w.full > lim.full) break;
	max_h = TMath::Max(max_h, j->fWeedInfo->fHeight);
	w += z; ii = j++; ++n_join;
      }
    }

    if(n_join > 1) { // Set multiple weeds
      int  taken = 0;
      int  spare = lim.full - w.full;
      int  align = FL_ALIGN_LEFT;
      bool flip_align = (n_join==2 && vs.bRightLabelPair);
      int  n_join_org = n_join;
      while(i != j) {
	GVNS::WeedInfo& mi = *i->fWeedInfo;
	MTW_Widths z = i->GetWidths(cell_w);
	int dw;
	if(n_join_org == 2) {
	  int ideal = int( roundf(((float)(lim.full - taken))/n_join) );
	  dw = (ideal - z.full < spare) ? ideal - z.full : spare;
	} else {
	  dw = spare / n_join;
	}
	spare -= dw;
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
      GVNS::WeedInfo& mi = *i->fWeedInfo;
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

OS::ZLinkDatum* MTW_SubView::GrepLinkDatum(const TString& link_fqn)
{
  if(mView->fImg != 0) {
    OS::lZLinkDatum_t& lds = mView->fImg->fLinkData;
    for(OS::lZLinkDatum_i i=lds.begin(); i!=lds.end(); ++i)
      if(i->GetLinkInfo()->FullName() == link_fqn)
	return &(*i);
  }
  return 0;
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
// MTW_Weed
/**************************************************************************/

MTW_Widths MTW_Weed::GetWidths(int cell_w)
{
  MTW_Widths ret;
  ret.weed = fWeedInfo->fWidth;
  if(fWeedInfo->bLabel) {
    if(fWeedInfo->bLabelInside) {
      ret.weed += FGS::swm_string_width(fWeedInfo->fName, cell_w);
      ret.label = 0;
    } else {
      ret.label = FGS::swm_label_width(fWeedInfo->fName, cell_w);
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
  float align_fac = TMath::Min
    ( max_align_grow, float(fMaxLabelW + fMaxOutsideLabeledW)/fMaxFullW );
  float join_fac  = TMath::Min
    ( max_join_grow, float(fMaxGreedyJoinW)/fMaxFullW );

  if(align_fac>1 || join_fac>1) {
    float grow_fac = TMath::Max(align_fac, join_fac);
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
