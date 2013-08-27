// $Id$

#include "FTW_SubShell.h"
#include <FL/Fl_Window.H>

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// FTW_SubShell
//

FTW_SubShell::FTW_SubShell(FTW_Shell* s, Fl_Window* w, Fl_Widget* c) :
  FTW_ShellClient(s),
  mWindow(w),
  mContents(c)
{}

FTW_SubShell::~FTW_SubShell()
{
  if (mWindow != mContents)
    delete mWindow;
}

void FTW_SubShell::label_window(const char* l)
{
  mWindowLabel = l;
  mWindow->label(mWindowLabel.Data());
  mWindow->redraw();
}

void FTW_SubShell::dock(Fl_Group* g)
{
  // Dock into group, taking its full size.

  mWindow->hide();
  if(mWindow != mContents) {
    mContents->hide();
  }
  g->add(*mContents);
  mContents->resize(g->x(), g->y(), g->w(), g->h());
  mContents->show();
}

void FTW_SubShell::undock()
{
  mContents->hide();
  if(mWindow != mContents) {
    mWindow->add(mContents);
    mContents->resize(0, 0, mWindow->w(), mWindow->h());
    mContents->show();
  } else if (mContents->parent()) {
    mContents->parent()->remove(mContents);
  }
  mWindow->free_position();
  mWindow->show();
}
