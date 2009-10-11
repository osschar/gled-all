// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.


#ifndef Fl_SVars_H
#define Fl_SVars_H

#include <FL/Fl_Group.H>
#include <FL/Fl_Valuator.H>

#include <TString.h>

//==============================================================================
// Fl_SVar
//==============================================================================

class Fl_SVar : public Fl_Group
{
protected:
  Fl_Valuator *mV;
  TString      mVTip;

public:
  Fl_SVar(int x, int y, int w,int h, const char* l=0);

  void   minimum(double m)  { mV->minimum(m); }
  void   maximum(double M)  { mV->maximum(M); }

  double step() const       { return mV->step(); }
  void   step(double s)     { mV->step(s);    }
  void   step(int A, int B) { mV->step(A, B); }

  double value()      const { return mV->value(); }
};


//==============================================================================
// Fl_SMinMaxVar
//==============================================================================

class Fl_SMinMaxVar : public Fl_SVar
{
protected:

public:
  Fl_SMinMaxVar(int x, int y, int w,int h, const char* l=0);

  void set(double v, double min, double max);

  virtual void resize(int x, int y, int w, int h);
};


//==============================================================================
// Fl_SDesireVar
//==============================================================================

class Fl_SDesireVar : public Fl_SVar
{
protected:
  Fl_Valuator *mTV; // TrueValue (mV is desire in this class)

public:
  Fl_SDesireVar(int x, int y, int w,int h, const char* l=0);

  void set(double v, double min, double max, double desire);

  void step(double s)     { mV->step(s); mTV->step(s); }
  void step(int A, int B) { mV->step(A, B); mTV->step(A, B); }

  virtual void resize(int x, int y, int w, int h);
};

#endif
