// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_MTW_ClassView_H
#define GledCore_MTW_ClassView_H

#include "MTW_View.h"
#include <FL/Fl_Pack.H>

class MTW_Layout;

class MTW_ClassView : public MTW_View, public Fl_Pack
{
private:
  void _init();

protected:
  // virtual void auto_label(); use default labeling

public:
  // View created from FTW_Shell:
  MTW_ClassView(OptoStructs::ZGlassImg* img, FTW_Shell* shell);
  // Direct view for non enlightened lenses:
  MTW_ClassView(ZGlass* glass, Fl_SWM_Manager* swm_mgr);
  virtual ~MTW_ClassView();

  void BuildVerticalView();
  void BuildByLayout(MTW_Layout* layout);

  virtual int handle(int ev);

#include "MTW_ClassView.h7"
}; // endclass MTW_ClassView

#endif
