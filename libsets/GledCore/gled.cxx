// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

// This file contains main() for gled.

#include <Gled/GledTypes.h>
#include <Gled/GledNS.h>
#include <Gled/Gled.h>
#include <GledView/GledGUI.h>
#include <Gled/GThread.h>

#include <TRint.h>
#include <Getline.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifndef __APPLE__
#include <X11/Xlib.h>
#endif

/**************************************************************************/

int main(int argc, char **argv)
{
  static const Exc_t _eh("gled::main() ");

  // This is not so beautiful, but needs to be called first.
  // See also the X include above.
  // This is principle called from TApplication constructor, we call it
  // much later - in principle should really sub-class Gled from TRint or
  // have a TRint derived class as i used to (Gint).
  // Also - GledGUI should NOT inherit from Fl_Window.
  // !!!!
#ifndef __APPLE__
  if (XInitThreads() == 0)
  {
    fprintf(stderr, "XThreads not enabled ... might get into a mess.\n");
  }
#endif

  GThread::InitMain();

  GledGUI* gled = new GledGUI();
  gled->ReadArguments(argc, argv);
  gled->ParseArguments();

  if (gled->GetQuit())
  {
    exit(0);
  }

  gled->InitLogging();
  gled->InitGledCore();

  // Run TRint
  GThread app_thread("gled.cxx-TRintRunner",
                     (GThread_foo) Gled::TRint_runner_tl, 0, false);
  if (app_thread.Spawn())
  {
    perror(GForm("%scan't create Rint thread.", _eh.Data()));
    exit(1);
  }

  // Run Gled ... FLTK event loop for GledGUI. [ This is somewhat silly. ]
  gled->Run();

  if (gled->GetRintRunning())
  {
    app_thread.Kill(GThread::SigTERM);
  }
  else
  {
    Getlinem(kCleanUp, 0);
  }
  app_thread.Join();

  gled->StopLogging();
  delete gled;

  GThread::FiniMain();

  exit(0);
}
