// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_FTW_SubShell_H
#define GledCore_FTW_SubShell_H

class FTW_Shell;
class Fl_Window;

class FTW_SubShell {
protected:
  FTW_Shell*	mShell;
  Fl_Window*	mWindow; // Needed due to a braindead bug in gcc-3.2.3.

public:
  FTW_SubShell(FTW_Shell* s=0) : mShell(s), mWindow(0) {}

  FTW_Shell* GetShell()          { return mShell;  }
  virtual Fl_Window* GetWindow() { return mWindow; }
}; // endclass FTW_SubShell

#endif
