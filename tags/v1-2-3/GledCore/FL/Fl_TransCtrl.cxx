// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.


#include "Fl_TransCtrl.h"

typedef float Float_t; // This sux, but don't want to import ROOT stuff

/**************************************************************************/

namespace {
  void pos_rot_cb(Fl_Widget* o, Fl_Widget* m)
  { m->do_callback(); }

  bool df_cmp(double a, double b)
  { return (Float_t) a==(Float_t) b; }

  char *pos_labels[] = { "x", "y", "z" };
  char *rot_labels[] = { "a", "b", "c" };
}

/**************************************************************************/
// Fl_TransPosCtrl

Fl_TransPosCtrl::Fl_TransPosCtrl(int x, int y, int w,int h, const char* l) :
  Fl_Group(x,y,w,h,l)
{
  if(w<3) w = 3; // at least labels; expect resize
  int iw = (w - 3) / 3;
  int rw = (w - 3) % 3;
  
  int xc = x;
  for(int i=0; i<3; ++i) {
    int tw = iw; if(rw>0) { ++tw; --rw; }
    mP[i] = new Fl_Value_Input(xc+1, y, tw, h);
    mP[i]->label(pos_labels[i]);
    mP[i]->align(FL_ALIGN_LEFT);
    mP[i]->step(1,1000); mP[i]->range(-1e6, 1e6);
    mP[i]->callback((Fl_Callback*)pos_rot_cb, (void*)this);
    x += tw + 1;
  }

  box(FL_FLAT_BOX);
  end();
  resizable(0);
}

void Fl_TransPosCtrl::pos(double x, double y, double z)
{
  if(!df_cmp(x, mP[0]->value())) mP[0]->value((Float_t)x);
  if(!df_cmp(y, mP[1]->value())) mP[1]->value((Float_t)y);
  if(!df_cmp(z, mP[2]->value())) mP[2]->value((Float_t)z);
}

void Fl_TransPosCtrl::resize(int x, int y, int w, int h)
{
  if(Fl_Widget::w()==0 && Fl_Widget::h()==0) {
    int iw = (w - 3) / 3;
    int rw = (w - 3) % 3;
    int xc = x;
    for(int i=0; i<3; ++i) {
      int tw = iw; if(rw>0) { ++tw; --rw; }
      mP[i]->resize(xc+1, y, tw, h);
      xc += tw + 1;
    }
    Fl_Widget::resize(x, y, w, h);
  } else {
    Fl_Group::resize(x, y, w, h);
  }
}

/**************************************************************************/
// Fl_TransRotCtrl

const double Fl_TransRotCtrl::d2r = 1.745329251994329576e-2;

Fl_TransRotCtrl::Fl_TransRotCtrl(int x, int y, int w,int h, const char* l) :
  Fl_Group(x,y,w,h,l)
{
  if(w<3) w = 3; // at least labels; expect resize
  int iw = (w - 3) / 3;
  int rw = (w - 3) % 3;
  
  int xc = x;
  for(int i=0; i<3; ++i) {
    int tw = iw; if(rw>0) { ++tw; --rw; }
    mP[i] = new Fl_Value_Input(xc+1, y, tw, h);
    mP[i]->label(rot_labels[i]);
    mP[i]->align(FL_ALIGN_LEFT);
    mP[i]->step(1,100); mP[i]->range(-1e4, 1e4); // Range is just for dragging
    mP[i]->callback((Fl_Callback*)pos_rot_cb, (void*)this);
    xc += tw + 1;
  }

  box(FL_FLAT_BOX);
  end();
  resizable(0);
}

void Fl_TransRotCtrl::rot(double x, double y, double z)
{
  double q;
  q = mungl(x); if(!df_cmp(q, mP[0]->value())) mP[0]->value(q);
  q = mungl(y); if(!df_cmp(q, mP[1]->value())) mP[1]->value(q);
  q = mungl(z); if(!df_cmp(q, mP[2]->value())) mP[2]->value(q);
}

void Fl_TransRotCtrl::resize(int x, int y, int w, int h)
{
  if(Fl_Widget::w()==0 && Fl_Widget::h()==0) {
    int iw = (w - 3) / 3;
    int rw = (w - 3) % 3;
    int xc = x;
    for(int i=0; i<3; ++i) {
      int tw = iw; if(rw>0) { ++tw; --rw; }
      mP[i]->resize(xc+1, y, tw, h);
      xc += tw + 1;
    }
    Fl_Widget::resize(x, y, w, h);
  } else {
    Fl_Group::resize(x, y, w, h);
  }
}
