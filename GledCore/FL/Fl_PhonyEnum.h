// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Fl_PhonyEnum
#define GledCore_Fl_PhonyEnum

#include <FL/Fl_Choice.H>
#include <map>

class Fl_PhonyEnum : public Fl_Choice {
protected:
  std::map<int,int>	mVal2Idx;
  bool			bOwnMap;
  int			mTrueVal; // X{gs}
  int			mHiIdx;

public:
  Fl_PhonyEnum(int x=0, int y=0, int w=200,int h=80, const char* l= 0);

  void Bruh(int val, const char* label);
  void Update(int val);

#include "Fl_PhonyEnum.h7"

}; // endclass Fl_PhonyEnum

#endif
