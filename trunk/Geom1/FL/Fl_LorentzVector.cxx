// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.


#include "Fl_LorentzVector.h"
#include <math.h>

/**************************************************************************/

namespace {
  void master_cb(Fl_Widget* o, Fl_Widget* m)
  { m->do_callback(); }

  char *sub_labels[] = { "t", "x", "y", "z" };
}

/**************************************************************************/
// Fl_LorentzVector

Fl_LorentzVector::Fl_LorentzVector(int x, int y, int w,int h, const char* l) :
  Fl_Group(x,y,w,h,l)
{
  if(w<4) w = 4; // at least labels; expect resize
  int iw = (w - 4) / 4;
  int rw = (w - 4) % 4;
  
  int xc = x;
  for(int i=0; i<4; ++i) {
    int tw = iw; if(rw>0) { ++tw; --rw; }
    mP[i] = new Fl_Value_Input(xc+1, y, tw, h);
    mP[i]->label(sub_labels[i]);
    mP[i]->align(FL_ALIGN_LEFT);
    mP[i]->step(1,1000); mP[i]->range(-1e6, 1e6);
    mP[i]->callback((Fl_Callback*)master_cb, (void*)this);
    x += tw + 1;
  }

  end();
  resizable(0);
}

void Fl_LorentzVector::resize(int x, int y, int w, int h)
{
  if(Fl_Widget::w()==0 && Fl_Widget::h()==0) {
    int iw = (w - 4) / 4;
    int rw = (w - 4) % 4;
    int xc = x;
    for(int i=0; i<4; ++i) {
      int tw = iw; if(rw>0) { ++tw; --rw; }
      mP[i]->resize(xc+1, y, tw, h);
      xc += tw + 1;
    }
    Fl_Widget::resize(x, y, w, h);
  } else {
    Fl_Group::resize(x, y, w, h);
  }
}

