// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// MCW_Button
//
// A smart MethodCall button. P7 name 'MCWButt'.
//
// Marks the name by '...' if method-call-view needs to spawned (M1 or
// M3); by '..' if the method can be called directly as well
// (M1-direct call with default arguments, M3-spawn MTW_View).
//
// Accepts drag-in and paste events if the method takes lens-args and
// all other args have default values (marked by [] in front of method
// name and slightly bluish color).

#include "MCW_Button.h"
#include "MCW_View.h"
#include <GledView/FTW_Shell.h>

#include <FL/Fl.H>

namespace OS   = OptoStructs;
namespace GNS  = GledNS;

MCW_Button::MCW_Button(OS::ZGlassImg* img, Data& dt,
		       int x, int y, int w, int h, const char* t):
  Fl_Button(x,y,w,h,t), fImg(img), fData(dt)
{
  if(fImg == 0) deactivate();
  label(fData.fLabel.Data());
  if(fData.fDndP) labelcolor(fl_rgb_color(0,10,50));
  tooltip(fData.fTooltip.Data());
}

/**************************************************************************/

void MCW_Button::FillData(GNS::MethodInfo* mi, const char* label,
			  MCW_Button::Data& dt)
{
  dt.fMInfo = mi;
  dt.fLabel = "";
  dt.fTooltip = mi->fName + "(";

  lStr_t args;

  int nc=0, ncd=0, na=0, nad=0;
  TString t, n, d;
  for(lStr_i m=mi->fContextArgs.begin(); m!=mi->fContextArgs.end(); ++m) {
    GNS::split_argument(*m, t, n, d);
    if( ! d.IsNull()) ++ncd;
    ++nc;
    args.push_back(*m);
  }
  for(lStr_i m=mi->fArgs.begin(); m!=mi->fArgs.end(); ++m) {
    GNS::split_argument(*m, t, n, d);
    if( ! d.IsNull()) ++nad;
    ++na;
    args.push_back(*m);
  }

  if(ncd==nc && nad==na)
    dt.fDirectP = true;
  if((nc==1 || (nc>1 && ncd==nc)) && (nad==na))
    dt.fDndP = true;

  dt.fLabel  = (dt.fDndP) ? "[]  " : "";
  dt.fLabel += label;
  dt.fLabel += (dt.fDirectP) ? " .." : " ...";

  dt.fTooltip += GNS::join_strings(", ", args) + ")";
}

/**************************************************************************/

int MCW_Button::handle(int ev)
{
  static const Exc_t _eh("MCW_Button::handle ");

  switch(ev) {

  case FL_PUSH: {
    if(Fl::event_button() == 2 && fData.fDndP) {
      Fl::paste(*this);
    }
    break;
  }

  case FL_RELEASE: {
    if(Fl::event_button() == 2 && fData.fDndP) {
      break;
    }
    if(Fl::event_inside(this)) {
      FTW_Shell* shell = dynamic_cast<FTW_Shell*>(fImg->fEye);
      if(shell && Fl::event_button() == 1 && fData.fDirectP) {
	auto_ptr<MCW_View> v( shell->MakeMCW_View(fImg, fData.fMInfo) );
	if(v.get() != 0) {
	  v->SetABG(fImg->fLens);
	  v->Send();
	}
      } else {
	shell->SpawnMCW_View(fImg, fData.fMInfo);
      }
    }
    break;
  }

  case FL_DND_ENTER:   return 1;
  case FL_DND_RELEASE: return (Fl::event_inside(this)) ? 1 : 0;
  case FL_DND_LEAVE:   return 1;

  case FL_PASTE: {
    FTW_Shell *shell = dynamic_cast<FTW_Shell*>(fImg->fEye);
    if(shell == 0) return 0;
    ID_t beta_id = shell->GetSource()->get_contents();
    if(beta_id != 0) {
      auto_ptr<MCW_View> v( shell->MakeMCW_View(fImg, fData.fMInfo) );
      if(v.get() != 0) {
	v->SetABG(fImg->fLens->GetSaturnID(), beta_id);
	v->Send();
      }
    }
    return 1;
  }
  } // end switch(ev)

  return Fl_Button::handle(ev);
}
