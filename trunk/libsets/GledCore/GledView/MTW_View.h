// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_MTW_View_H
#define GledCore_MTW_View_H

#include "FTW_ShellClient.h"

#include <Gled/GledTypes.h>
#include <Eye/OptoStructs.h>

#include <FL/Fl_SWM.H>
#include <FL/Fl_Box.H>

class MTW_SubView;
class ZGlass;
class Eye;

typedef list<MTW_SubView*>		lpMTW_SubView_t;
typedef list<MTW_SubView*>::iterator	lpMTW_SubView_i;

class Fl_SWM_Manager;

class MTW_View : public OptoStructs::A_View,
		 public Fl_SWM_Client,
		 public FTW_ShellClient
{
private:
  void _init();

protected:
  TString  m_window_label;
  virtual void auto_label();

  ZGlass*		mGlass;
  lpMTW_SubView_t	mSubViews;

  Fl_Window*            mWindow;
  Fl_Group*		mFltkRep;
  bool			bShown;
  bool                  bManaged;

  void set_window(Fl_Window* win);

  static void mtw_view_closed(Fl_Window* win, MTW_View* mtw_view);

  // Self representation
  class SelfRep : public Fl_Box
  {
    MTW_View* fView;
  public:
    SelfRep(MTW_View* v, int x, int y, int w, int h);
    virtual int handle(int ev);
  };

  SelfRep*		mSelfRep;

public:
  // View created from FTW_Shell:
  MTW_View(OptoStructs::ZGlassImg* img, FTW_Shell* shell);
  // Direct view for non enlightened lenses:
  MTW_View(ZGlass* glass, Fl_SWM_Manager* swm_mgr);
  ~MTW_View();

          FTW_Shell* GetShell()  const { return mShell; }
  virtual Fl_Window* GetWindow() const { return mWindow; }

  void SetManaged(bool m) { bManaged = m; }

  void Labelofy();

  virtual void AbsorbRay(Ray& ray);
  virtual void AssertDependantViews() {}

  virtual void InvalidateRnrScheme() {}

  void UpdateDataWeeds(FID_t fid);
  void UpdateLinkWeeds(FID_t fid);

#include "MTW_View.h7"
}; // endclass MTW_View

#endif
