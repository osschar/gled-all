// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_MTW_SubView
#define Gled_MTW_SubView

#include <Gled/GledTypes.h>
#include <Gled/GledNS.h>
#include <GledView/GledViewNS.h>
#include <FL/Fl_RelGroup.H>

class Eye;
class MTW_SubView; class MTW_View;
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
  MTW_Widths GetWidths();
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

public:
  MTW_SubView(GledNS::ClassInfo* ci, MTW_View* v);
  virtual ~MTW_SubView();

  void Update();

  void BuildFromList(GledViewNS::lpWeedInfo_t& l);
  void UpdateVerticalStats(MTW_Vertical_Stats& vs);
  int  ResizeByVerticalStats(MTW_Vertical_Stats& vs);

  static const int MinLabelWidth;

}; // endclass MTW_SubView

#endif
