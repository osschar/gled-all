// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

// OK ... this just spawns Gled and CINT
// Should also parse command line options ...

#include <Gled/GledTypes.h>
#include <Gled/GledNS.h>
#include <Gled/Gled.h>
#include <Gled/GThread.h>
#include <Gled/GCondition.h>

#include <TROOT.h>
#include <TSystem.h>
#include <TRint.h>
#include <Getline.h>

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

VoidFuncPtr_t initfuncs[] = { 0 };

TRint*		gint;
Gled*		gled;
bool		gint_off = true;

/**************************************************************************/

void* RunRint(void* x) {
  gint_off = false;
  gint->TApplication::Run(true);
  gint_off = true;

  cout << "Gint terminated ...\n";
  if(Gled::theOne->GetQuit()==false) Gled::theOne->Exit();
  return 0;
}

/**************************************************************************/

int main(int argc, char **argv)
{
  list<char*> args; for(int i=1; i<argc; ++i) args.push_back(argv[i]);

  TROOT root("Root", "ROOT of GLED", initfuncs);
  GledNS::GledRoot = new TDirectory("Gled", "Gled root directory");
  GledNS::InitFD(0, GledNS::GledRoot);
  gROOT->Time(0);
  // gROOT->SetStyle("Plain");
  gROOT->SetMacroPath(GForm(".:%s/.gled:%s/macros",
			    getenv("HOME"), getenv("GLEDSYS")));

  // Spawn Gled
  gled = new Gled(args);
  if(gled->GetQuit()) exit(0);

  // Prepare remaining args for ROOT, weed out remaining options
  int   rargc = 1;
  char* rargv[args.size() + 4];
  rargv[0] = argv[0];
  rargv[rargc++] = "-q";
  rargv[rargc++] = "-b"; // batch-mode, no graphics
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

  // Prepare gled exit condition & Spawn saturn
  GCondition gled_exit;
  gled_exit.Lock();
  gled->SetExitCondVar(&gled_exit);
  if(gled->GetAutoSpawn()) {
    gled->SpawnSunOrSaturn();  
  }

  // Spawn TRint
  if(gled->GetShowSplash())
    cout <<"Staring ROOT command-line interpreter ...\n";
  gint = new TRint("TRint", &rargc, rargv);
  gint->SetPrompt("saturn[%d] ");

  // Process macros;
  // -q added to options, so it exits after macro processing
  gint->Run(true); // Process macros
  if(gint->InputFiles()) {
    gint->ClearInputFiles();
  } else {
    Getlinem(kCleanUp, 0);
  }
  Getlinem(kInit, gint->GetPrompt());

  if(gled->GetAllowMoons())
    gled->AllowMoonConnections();


  // Run TRint
  GThread app_thread(RunRint, 0, false);
  if(gled->GetRunRint()) {
    if( app_thread.Spawn() ) {
      perror("saturn.cxx can't create Rint thread");
      exit(1);
    }
  }

  gled_exit.Wait();
  gled_exit.Unlock();
  if(!gint_off) {
    gint->Terminate(0);
    app_thread.Kill(GThread::SigUSR1);
  } else {
    Getlinem(kCleanUp, 0);
  }
  app_thread.Join();

  delete gint;
  delete gled;

  exit(0);
}
