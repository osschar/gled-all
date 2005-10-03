// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_FTW_SubShell_H
#define GledCore_FTW_SubShell_H

#include <TString.h>

class FTW_Shell;
class Fl_Window;

class FTW_SubShell {
protected:
  FTW_Shell*    mShell;
  Fl_Window*    mWindow;

  TString       mWindowLabel;

public:
  FTW_SubShell(FTW_Shell* s=0, Fl_Window* w=0) : mShell(s), mWindow(w) {}
  virtual ~FTW_SubShell() {}

  FTW_Shell* GetShell()          { return mShell;  }
  virtual Fl_Window* GetWindow() { return mWindow; }

  virtual void label_window(const char* l=0);

}; // endclass FTW_SubShell

#endif
