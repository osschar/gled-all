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

#include <TROOT.h>
#include <TSystem.h>
#include <TThread.h>
#include <TRint.h>
#include <TInterpreter.h>
#include <Getline.h>

#include <X11/Xlib.h>

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

TRint*		gint;
GledGUI*	gled;

/**************************************************************************/

int main(int argc, char **argv)
{
  static const Exc_t _eh("gled::main() ");

  list<char*> args; for(int i=1; i<argc; ++i) args.push_back(argv[i]);

  if(XInitThreads() == 0) {
    cerr << "XThreads not enabled ... might get into a mess ...\n";
  }

  GledNS::GledRoot = new TDirectory("Gled", "Gled root directory");
  GledNS::InitFD(0, GledNS::GledRoot);
  gROOT->Time(0);
  // gROOT->SetStyle("Plain");
  gSystem->Load("libThread");
  TThread init_root_threads;
  gROOT->SetMacroPath(GForm(".:%s/.gled:%s/macros",
			    getenv("HOME"), getenv("GLEDSYS")));
  gInterpreter->AddIncludePath(GForm("%s/.gled", getenv("HOME")));
  gInterpreter->AddIncludePath(GForm("%s/macros", getenv("GLEDSYS")));
  gInterpreter->SetProcessLineLock(false);

  // Spawn Gled
  gled = new GledGUI();
  gled->ParseArguments(args);
  if(gled->GetQuit()) exit(0);
  gled->InitLogging();

  // Prepare remaining args for ROOT, weed out remaining options
  int   rargc = 1;
  const char* rargv[args.size() + 3];
  rargv[0] = argv[0];
  // Enforce return from Run; trick root so that it only processes macros
  rargv[rargc++] = "-q";
  if(!gled->GetShowSplash())
    rargv[rargc++] = "-l";
  for(list<char*>::iterator i=args.begin(); i!=args.end(); ++i) {
    if((*i)[0] == '-') {
      cout <<"Ignoring option "<< *i <<endl;
    } else {
      rargv[rargc++] = *i;
    }
  }
  args.clear();

  // Init GledCore libset
  gled->InitGledCore();

  // Call pre-exec macros if any.
  if(gled->GetPreExec()) {
    gled->PreExec();
  }

  // Spawn TRint
  if(gled->GetShowSplash())
    cout <<"Staring ROOT command-line interpreter ...\n";
  gint = new TRint("TRint", &rargc, (char**) rargv);
  gint->SetPrompt("gled[%d] ");
  printf("Use context menu in Nest to import lenses as CINT variables.\n");

  // Run GUI
  GThread gled_thread((GThread_foo)Gled::Gled_runner_tl, gled, false);
  if( gled_thread.Spawn() ) {
    perror(GForm("%scan't create Gled thread", _eh.Data()));
    exit(1);
  }

  // Spawn saturn
  if(gled->GetAutoSpawn()) {
    gled->SpawnSunOrSaturn();
  }

  // Process macros;
  // -q added to options, so it exits after macro processing
  try {
    gint->Run(true);
  }
  catch(exception& exc) {
    fprintf(stderr, "%sexception caught during macro processing:\n%s\n",
	    _eh.Data(), exc.what());
    exit(1);
  }
  if(gint->InputFiles()) {
    gint->ClearInputFiles();
  } else {
    Getlinem(kCleanUp, 0);
  }
  Getlinem(kInit, gint->GetPrompt());

  if(gled->GetAllowMoons())
    gled->AllowMoonConnections();

  // Run TRint
  GThread app_thread((GThread_foo)Gled::TRint_runner_tl, gint, false);
  if(gled->GetRunRint()) {
    if( app_thread.Spawn() ) {
      perror(GForm("%scan't create Rint thread.", _eh.Data()));
      exit(1);
    }
  }

  gled_thread.Join();
  if(gled->GetRintRunning()) {
    gint->Terminate(0);
    // It seems ROOT's thread does not like to be canceled anymore as it
    // exits with an uncought, unknown exception.
    // app_thread.Cancel();
    // Tried this, but it didn't wake up the event-loop.
    // gSystem->AddTimer(new TTimer());
  } else {
    Getlinem(kCleanUp, 0);
  }
  // Don't wait ... see above.
  // app_thread.Join();

  delete gint;
  gled->StopLogging();
  delete gled;

  exit(0);
}
