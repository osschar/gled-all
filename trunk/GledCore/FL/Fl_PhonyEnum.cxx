// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Fl_PhonyEnum.h"

void pe_cb(Fl_PhonyEnum* o, int v) {
  o->SetTrueVal(v);
  o->do_callback();
}

Fl_PhonyEnum::Fl_PhonyEnum(int x, int y, int w,int h, const char* l) :
  Fl_Choice(x,y,w,h,l), mHiIdx(-1)
{}

void Fl_PhonyEnum::Bruh(int val, const char* label)
{
  mHiIdx++;
  mVal2Idx[val] = mHiIdx;
  add(label, 0, (Fl_Callback*)pe_cb, (void*)val);
}

void Fl_PhonyEnum::Update(int v)
{
  std::map<int,int>::iterator i = mVal2Idx.find(v);
  mTrueVal = v;
  value(i->second);
}
