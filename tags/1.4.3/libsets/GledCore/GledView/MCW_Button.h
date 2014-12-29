// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_MCW_Button_H
#define GledCore_MCW_Button_H

#include <Gled/GledTypes.h>
#include <Gled/GledNS.h>
#include <Eye/OptoStructs.h>

#include <FL/Fl_Button.H>

class MCW_View;

class MCW_Button : public Fl_Button
{
public:
  struct Data
  {
    TString               fLabel;
    TString               fTooltip;
    GledNS::MethodInfo   *fMInfo;
    bool                  fDirectP;
    bool                  fDndP;

    Data() : fMInfo(0), fDirectP(false), fDndP(false) {}
  };

protected:
  OptoStructs::ZGlassImg *fImg;
  Data&                   fData;

public:
  MCW_Button(OptoStructs::ZGlassImg* img, Data& dt,
	     int x, int y, int w, int h, const char* t=0);

  static void FillData(GledNS::MethodInfo* mi, const char* label,  Data& d);

  virtual int handle(int ev);

}; // endclass MCW_Button

#endif
