// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_MTW_View_H
#define GledCore_MTW_View_H

#include <Gled/GledTypes.h>
#include "OptoStructs.h"
#include "FTW_SubShell.h"

#include <FL/Fl_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_SWM.H>

class MTW_SubView; class MTW_Layout;
class ZGlass;
class Eye;

typedef list<MTW_SubView*>		lpMTW_SubView_t;
typedef list<MTW_SubView*>::iterator	lpMTW_SubView_i;

class Fl_SWM_Manager;

class MTW_View : public OptoStructs::A_View,
		 public FTW_SubShell,
		 public Fl_Pack, public Fl_SWM_Client
{
private:
  void _init();

protected:
  void auto_label();
  string m_label;

  ZGlass*		mGlass;
  lpMTW_SubView_t	mSubViews;

  bool			bShown;

  Fl_Window*		mWindow;	// X{gs}
  
  // Some alignment flags, collapses per mid-class

public:
  // View created from FTW_Shell:
  MTW_View(OptoStructs::ZGlassImg* img, FTW_Shell* shell);
  // Direct view for non enlightened lenses:
  MTW_View(ZGlass* glass, Fl_SWM_Manager* swm_mgr);
  ~MTW_View();

  void BuildVerticalView();
  void BuildByLayout(MTW_Layout* layout);

  void Labelofy();

  virtual void AbsorbRay(Ray& ray);
  virtual void AssertDependantViews() {}

  virtual void InvalidateRnrScheme() {}

  void UpdateDataWeeds(FID_t fid);
  void UpdateLinkWeeds(FID_t fid);

  void ShowWindow();

  virtual int handle(int ev);

#include "MTW_View.h7"
}; // endclass MTW_View

#endif
