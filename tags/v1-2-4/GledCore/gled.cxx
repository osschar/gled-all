// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
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

extern void  InitGui();  // ROOT's GUI initializer
VoidFuncPtr_t initfuncs[] = { InitGui, 0 };

TRint*		gint;
GledGUI*	gled;

TROOT root("Root", "ROOT of GLED", initfuncs);

/**************************************************************************/

int main(int argc, char **argv)
{
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

  // Spawn Gled
  gled = new GledGUI();
  gled->ParseArguments(args);
  if(gled->GetQuit()) exit(0);
  gled->InitLogging();

  // Prepare remaining args for ROOT, weed out remaining options
  int   rargc = 1;
  char* rargv[args.size() + 3];
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

  // Spawn TRint
  if(gled->GetShowSplash())
    cout <<"Staring ROOT command-line interpreter ...\n";
  gint = new TRint("TRint", &rargc, rargv);
  gint->SetPrompt("gled[%d] ");
  printf("Use context menu in Nest to import lenses as CINT variables.\n");

  // Run GUI
  GThread gled_thread((GThread_foo)Gled::Gled_runner_tl, gled, false);
  if( gled_thread.Spawn() ) {
    perror("gled.cxx can't create Gled thread");
    exit(1);
  }  

  // Init GledCore libset
  gled->InitGledCore();

  // Spawn saturn
  if(gled->GetAutoSpawn()) {
    gled->SpawnSunOrSaturn();
  }

  // Process macros;
  // -q added to options, so it exits after macro processing
  gint->Run(true);
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
      perror("gled.cxx can't create Rint thread");
      exit(1);
    }
  }

  gled_thread.Join();
  if(gled->GetRintRunning()) {
    app_thread.Cancel();
    gint->Terminate(0);
  } else {
    Getlinem(kCleanUp, 0);
  }
  app_thread.Join();

  delete gint;
  gled->StopLogging();
  delete gled;

  exit(0);
}
