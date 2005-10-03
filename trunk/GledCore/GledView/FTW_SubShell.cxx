// $Header$

#include "FTW_SubShell.h"
#include <FL/Fl_Window.H>

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// FTW_SubShell
//

void FTW_SubShell::label_window(const char* l)
{
  mWindowLabel = l;
  mWindow->label(mWindowLabel.Data());
  mWindow->redraw();
}
