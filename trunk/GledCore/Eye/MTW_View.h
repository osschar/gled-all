// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_MTW_View_H
#define Gled_MTW_View_H

// Includes
#include <Gled/GledTypes.h>
#include "OptoStructs.h"
#include <FL/Fl_Pack.H>

class MTW_SubView; class MTW_Layout;
class ZGlass;
class Eye;

typedef list<MTW_SubView*>		lpMTW_SubView_t;
typedef list<MTW_SubView*>::iterator	lpMTW_SubView_i;

class MTW_View : public OptoStructs::A_GlassView, public Fl_Pack {
private:
  ZGlass*		mGlass;
  lpMTW_SubView_t	mSubViews;
  int			mWidth;		// X{G}
  int			mHeight;	// X{G}
  int			mSkipH;		// X{Gs}

  bool			bShown;
  
  // Some alignment flags, collapses per mid-class

public:
  MTW_View(OptoStructs::ZGlassImg* img); // View within an eye
  MTW_View(ZGlass* glass);  		 // Direct view
  ~MTW_View();

  void BuildVerticalView();
  void BuildByLayout(MTW_Layout* layout);

  virtual void Absorb_Change(LID_t lid, CID_t cid);
  virtual void AssertDependantViews() {}

  virtual void InvalidateRnrScheme() {}

  // void Retitle(); !! belongs to covering window !!
  void UpdateViews(LID_t lid=0, CID_t cid=0);

  static Fl_Window* ConstructVerticalWindow(OptoStructs::ZGlassImg* img);
  static Fl_Window* ConstructVerticalWindow(ZGlass* glass);

  virtual int handle(int ev);

#include "MTW_View.h7"
}; // endclass MTW_View

#endif
