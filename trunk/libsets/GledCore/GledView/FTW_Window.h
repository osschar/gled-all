// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_FTW_Window_H
#define GledCore_FTW_Window_H

#include <FL/Fl_Window.H>
#include <FL/Fl_SWM.H>

class FTW_Window : public Fl_Window, public Fl_SWM_Client
{
public:
  FTW_Window(int w, int h, const char* t=0);
  FTW_Window(int x, int y, int w, int h, const char *t=0);
};

#endif
