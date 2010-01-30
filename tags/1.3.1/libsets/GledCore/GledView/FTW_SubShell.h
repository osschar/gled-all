// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_FTW_SubShell_H
#define GledCore_FTW_SubShell_H

#include <TString.h>
#include "FTW_ShellClient.h"

class Fl_Window;
class Fl_Group;
class Fl_Widget;

class FTW_SubShell : public FTW_ShellClient
{
protected:
  Fl_Window*    mWindow;
  Fl_Widget*    mContents;

  TString       mWindowLabel;

public:
  FTW_SubShell(FTW_Shell* s, Fl_Window* w, Fl_Widget* c);
  virtual ~FTW_SubShell();

  Fl_Window* GetWindow() { return mWindow; }

  virtual void label_window(const char* l=0);

  virtual void dock(Fl_Group* g);
  virtual void undock();
}; // endclass FTW_SubShell

#endif
