// $Header$

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

#include <X11/Xlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

GledGUI*	gled;

/**************************************************************************/

int main(int argc, char **argv)
{
  static const Exc_t _eh("gled::main() ");

  if (XInitThreads() == 0)
  {
    fprintf(stderr, "XThreads not enabled ... might get into a mess.\n");
  }

  GThread::InitMain();

  Gled::InitStatics();

  gled = new GledGUI();

  lStr_t args; for (int i=1; i<argc; ++i) args.push_back(argv[i]);
  gled->ParseArguments(args);

  if (gled->GetQuit())
  {
    exit(0);
  }

  gled->InitLogging();
  gled->InitGledCore();

  // Run Gled ... FLTK event loop for GledGUI. [ This is somewhat silly. ]
  GThread gled_thread("gled.cxx-GledRunner",
                      (GThread_foo) Gled::Gled_runner_tl, gled,
                      false);
  if (gled_thread.Spawn())
  {
    perror(GForm("%scan't create Gled thread", _eh.Data()));
    exit(1);
  }

  gled->ProcessCmdLineMacros("gled", args);

  // Run TRint
  GThread app_thread("gled.cxx-TRintRunner",
                     (GThread_foo) Gled::TRint_runner_tl, gled->GetRint(),
                     false);
  if (gled->GetRunRint())
  {
    if (app_thread.Spawn())
    {
      perror(GForm("%scan't create Rint thread.", _eh.Data()));
      exit(1);
    }
  }

  gled_thread.Join();
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
