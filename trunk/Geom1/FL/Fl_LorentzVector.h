// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.


#ifndef Fl_LorentzVector_H
#define Fl_LorentzVector_H

#include <FL/Fl_Group.H>
#include <FL/Fl_Value_Input.H>

class Fl_LorentzVector : public Fl_Group {
protected:
  Fl_Value_Input*	mP[4];

public:
  Fl_LorentzVector(int x, int y, int w,int h, const char* l=0);

  double t() const { return mP[0]->value(); }
  double x() const { return mP[1]->value(); }
  double y() const { return mP[2]->value(); }
  double z() const { return mP[3]->value(); }

  void set(double t, double x, double y, double z) {
    if(t != mP[0]->value()) mP[0]->value(t);
    if(x != mP[1]->value()) mP[1]->value(x);
    if(y != mP[2]->value()) mP[2]->value(y);
    if(z != mP[3]->value()) mP[3]->value(z);
  }

  virtual void resize(int x, int y, int w, int h);
};

#endif
