// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "FltkGledStuff.h"
#include "Eye.h"
#include "FTW_Shell.h"

#include "MCW_View.h"

#include <Net/Ray.h>
#include <FL/Fl.H>
#include <FL/fl_draw.H>

namespace FGS  = FltkGledStuff;
namespace OS   = OptoStructs;
namespace GNS  = GledNS;
namespace GVNS = GledViewNS;

/**************************************************************************/
// Fl helpers
/**************************************************************************/

int FGS::swm_generick_width(string& str, int cell_w, float extra)
{
  if(cell_w) {
    int w=0, h=0;
    fl_measure(str.c_str(), w, h);
    float f = float(w)/cell_w + extra;
    w = int(f);
    return w + ((f-w < 0.1) ? 0 : 1);
  } else {
    return str.size();
  }
}

int FGS::swm_label_width(string& str, int cell_w)
{ return swm_generick_width(str, cell_w, 1.2); }

int FGS::swm_string_width(string& str, int cell_w)
{ return swm_generick_width(str, cell_w, 0.2); }

/**************************************************************************/
// FGS::LensNameBox
/**************************************************************************/

FGS::LensNameBox::LensNameBox(OS::ZGlassImg* i, int x, int y, int w, int h) :
  OS::A_View(0), Fl_Box(x,y,w,h)
{
  labeltype((Fl_Labeltype)GVNS::no_symbol_label);
  color(fl_rgb_color(224,224,224));
  ChangeImage(i);
}

FGS::LensNameBox::~LensNameBox()
{
  if(fImg) fImg->CheckOutFullView(this);
}

/**************************************************************************/

void FGS::LensNameBox::AbsorbRay(Ray& ray)
{
  if(ray.IsBasicChange()) auto_label();
}

void FGS::LensNameBox::ChangeImage(OS::ZGlassImg* new_img)
{
  if(new_img && !GNS::IsA(new_img->fGlass, fFID)) {
    new_img->fEye->GetShell()->Message
      (GForm("FGS::LensNameBox::ChangeImage argument '%s::%s' is not of required type '%s'.",
	     new_img->fClassInfo->fName.c_str(), new_img->fGlass->GetName(),
	     GNS::FindClassInfo(fFID)->fName.c_str())
      );
    return;
  }

  if(fImg) fImg->CheckOutFullView(this);
  fImg = new_img;
  if(fImg) fImg->CheckInFullView(this);
  auto_label();
}

/**************************************************************************/

void FGS::LensNameBox::auto_label()
{
  if(fImg) {
    GNS::ClassInfo*   ci = fImg->fClassInfo;
    GNS::LibSetInfo* lsi = GNS::FindLibSetInfo(ci->fFid.lid);
    set_label(fImg->fGlass->GetName());
    set_tooltip(GForm("%s::%s [%d]", lsi->fName.c_str(), ci->fName.c_str(),
		      fImg->fGlass->GetSaturnID()));
  } else {
    set_label("<null>");
    set_tooltip(0);
  }
  redraw();
}

int FGS::LensNameBox::handle(int ev)
{
  switch(ev) {

  case FL_DND_ENTER: {
    // could check if valid type, change cursor
    return 1;
  }

  case FL_DND_RELEASE: {
    return (Fl::belowmouse() == this) ? 1 : 0;
  }

  case FL_DND_LEAVE: {
    // restore cursor
    return 1;
  }

  case FL_PASTE: {
    MCW_View* mcw = grep_parent<MCW_View*>(this);
    try {
      ID_t source_id = mcw->GetShell()->GetSource()->get_contents();
      ID_t paste_id = atoll(Fl::event_text());

      if(source_id == paste_id) {
	ChangeImage(mcw->GetShell()->GetEye()->DemangleID(paste_id));
      } else {
	throw(string("FGS::LensNameBox::handle dnd error."));
      }

    }
    catch(string exc) {
      mcw->GetShell()->Message(exc.c_str(), FTW_Shell::MT_err);
    }
    return 1;
  }
  } // end switch(ev)

  return Fl_Box::handle(ev);
}
