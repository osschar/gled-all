// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.


#ifndef Fl_TransCtrl_H
#define Fl_TransCtrl_H

#include <FL/Fl_Group.H>
#include <FL/Fl_Value_Input.H>

#include <math.h>

class Fl_TransPosCtrl : public Fl_Group {
protected:
  Fl_Value_Input*	mP[3];

public:
  Fl_TransPosCtrl(int x, int y, int w,int h, const char* l=0);

  double x() const { return mP[0]->value(); }
  double y() const { return mP[1]->value(); }
  double z() const { return mP[2]->value(); }

  void pos(double x, double y, double z);

  virtual void resize(int x, int y, int w, int h);
};

class Fl_TransRotCtrl : public Fl_Group {
protected:
  Fl_Value_Input*	mP[3];

public:
  static const double d2r;

  Fl_TransRotCtrl(int x, int y, int w,int h, const char* l=0);

  double phi() const { return mP[0]->value() * d2r; }
  double theta() const { return mP[1]->value() * d2r; }
  double eta() const { return mP[2]->value() * d2r; }

  void rot(double x, double y, double z);

  double mungl(double x) { return (int)round(100*x/d2r)/100.0; }

  virtual void resize(int x, int y, int w, int h);
};

#endif
