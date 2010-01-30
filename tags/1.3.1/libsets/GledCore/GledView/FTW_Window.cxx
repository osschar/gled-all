// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "FTW_Window.h"

FTW_Window::FTW_Window(int w, int h, const char* t) :
  Fl_Window(w,h,t),
  Fl_SWM_Client()
{
  resizable(this);
}
