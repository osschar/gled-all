// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_MTW_SubView_H
#define GledCore_MTW_SubView_H

#include <Gled/GledTypes.h>
#include <Gled/GledNS.h>
#include <GledView/GledViewNS.h>
#include <FL/Fl_RelGroup.H>

class Eye;
class MTW_SubView; class MTW_View;
namespace OptoStructs {
  class ZLinkDatum;
}

class Fl_Widget;

struct MTW_Widths {
  int	full, weed, label;
  MTW_Widths(int i=0, int j=0, int k=0) : full(i), weed(j), label(k) {}
  MTW_Widths& operator+=(MTW_Widths& r);
};

struct MTW_Vertical_Stats {
  int   	fMaxFullW, fMaxGreedyJoinW;
  int		fMaxOutsideLabeledW, fMaxLabelW;
  bool		bRightLabelPair;
  MTW_Widths	fUse;

  MTW_Vertical_Stats();
  void Consolidate(float max_align_grow, float max_join_grow);
  void Dump();
};

struct MTW_Weed {
  Fl_Widget*			fWeed;
  GledViewNS::WeedInfo*		fWeedInfo;

  MTW_Weed(Fl_Widget* wee, GledViewNS::WeedInfo* wi) :
    fWeed(wee), fWeedInfo(wi) {}
  MTW_Widths GetWidths(int cell_w);
};

typedef list<MTW_Weed>			lMTW_Weed_t;
typedef list<MTW_Weed>::iterator	lMTW_Weed_i;

/**************************************************************************/
// MTW_SubView
/**************************************************************************/

class MTW_SubView : public Fl_RelGroup {
  friend class MTW_View;

protected:
  GledNS::ClassInfo*	mClassInfo;
  MTW_View*		mView;	// Access to ZGlassImg, Eye
  lMTW_Weed_t		mWeeds;
  lMTW_Weed_t		mDataWeeds;
  lMTW_Weed_t		mLinkWeeds;

public:
  MTW_SubView(GledNS::ClassInfo* ci, MTW_View* v);
  virtual ~MTW_SubView();

  Fl_Widget* CreateWeed(GledViewNS::WeedInfo* wi);

  void Update();
  void UpdateDataWeeds();
  void UpdateLinkWeeds();

  void SetUpdateTimer();
  void RemoveUpdateTimer();
  static void UpdateFromTimer_s(MTW_SubView* v);

  void BuildFromList(GledViewNS::lpWeedInfo_t& l, bool show_links);
  void UpdateVerticalStats(MTW_Vertical_Stats& vs, int cell_w);
  int  ResizeByVerticalStats(MTW_Vertical_Stats& vs, int cell_w);

  OptoStructs::ZLinkDatum* GrepLinkDatum(const string& link_fqn);
  MTW_View* GetView() { return mView; }

  static const int MinLabelWidth;

}; // endclass MTW_SubView

#endif
