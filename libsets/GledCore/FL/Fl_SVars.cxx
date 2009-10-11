// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Fl_SVars.h"

#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Value_Output.H>

/**************************************************************************/

namespace
{
  void master_cb(Fl_Widget* o, Fl_Widget* m)
  { m->do_callback(); }
}


//==============================================================================
// Fl_SVar
//==============================================================================

Fl_SVar::Fl_SVar(int x, int y, int w, int h, const char* l) :
  Fl_Group(x,y,w,h,l)
{
  // This is meant as a base-class => the group end() is not called.

  resizable(this);
}


//==============================================================================
// Fl_SMinMaxVar
//==============================================================================

Fl_SMinMaxVar::Fl_SMinMaxVar(int x, int y, int w, int h, const char* l) :
  Fl_SVar(x,y,w,h,l)
{
  mV = new Fl_Value_Input(x, y, w, h);
  mV->callback((Fl_Callback*)master_cb, (void*)this);
  end();
}

void Fl_SMinMaxVar::set(double v, double min, double max)
{
  mV->value(v);
  mV->range(min, max);
  mVTip.Form("Range: %f - %f", min, max);
  mV->tooltip(mVTip.Data());
}

void Fl_SMinMaxVar::resize(int x, int y, int w, int h)
{
  if (Fl_Widget::w() == 0 && Fl_Widget::h() == 0)
  {
    Fl_Widget::resize(x, y, w, h);
    mV->resize(x, y, w, h);
  }
  else
  {
    Fl_Group::resize(x, y, w, h);
  }
}


//==============================================================================
// Fl_SDesireVar
//==============================================================================

Fl_SDesireVar::Fl_SDesireVar(int x, int y, int w, int h, const char* l) :
  Fl_SVar(x,y,w,h,l)
{
  int wh = w / 2;
  mV  = new Fl_Value_Input(x, y, wh, h);
  mV->callback((Fl_Callback*)master_cb, (void*)this);
  mTV = new Fl_Value_Output(x+wh, y, w-wh, h);
  end();
}

void Fl_SDesireVar::set(double v, double min, double max, double desire)
{
  mV->value(desire);
  mV->range(min, max);
  mVTip.Form("Range: %f - %f", min, max);
  mV->tooltip(mVTip.Data());

  mTV->value(v);
}

void Fl_SDesireVar::resize(int x, int y, int w, int h)
{
  if (Fl_Widget::w() == 0 && Fl_Widget::h() == 0)
  {
    int wh = w / 2;
    Fl_Widget::resize(x, y, w, h);
    mV->resize(x, y, wh, h);
    mTV->resize(x+wh, y, w-wh, h);
  }
  else
  {
    Fl_Group::resize(x, y, w, h);
  }
}
