// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_MTW_MetaView_H
#define GledCore_MTW_MetaView_H

#include "MTW_View.h"
#include <FL/Fl_Group.H>

class MetaSubViewInfo;

class MTW_MetaView : public MTW_View, public Fl_Group
{
private:
  void _init();

protected:
  // virtual void auto_label(); use default labeling

  ID_t	mGuiTemplate; // X{GS}

  void build_subview(MTW_SubView* sv, MetaSubViewInfo* msvi,
		     Int_t cX, Int_t cY);

public:
  MTW_MetaView(OptoStructs::ZGlassImg* img, FTW_Shell* shell);
  MTW_MetaView(ZGlass* glass, Fl_SWM_Manager* swm_mgr);
  virtual ~MTW_MetaView();

  void BuildByLensGraph(ZGlass* gui);
  void Rebuild();

  virtual int handle(int ev);

#include "MTW_MetaView.h7"
}; // endclass MTW_MetaView

#endif
