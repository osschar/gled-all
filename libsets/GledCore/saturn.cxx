// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

// saturn ... non-GUI gled (just spawns Gled and CINT)
//	      usefull for pure servers / proxies or computing clients

#include <Gled/GledTypes.h>
#include <Gled/GledNS.h>
#include <Gled/Gled.h>
#include <Gled/GThread.h>
#include <Gled/GCondition.h>

#include <TRint.h>
#include <Getline.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

/**************************************************************************/

int main(int argc, char **argv)
{
  static const Exc_t _eh("saturn::main() ");

  GThread::InitMain();

  Gled* gled = new Gled();
  gled->ReadArguments(argc, argv);
  gled->ParseArguments();

  if (gled->GetQuit())
  {
    exit(0);
  }

  gled->InitLogging();
  gled->InitGledCore();

  GCondition gled_exit;
  gled_exit.Lock();
  gled->SetExitCondVar(&gled_exit);

  // Run TRint
  GThread app_thread("saturn.cxx-TRintRunner",
                     (GThread_foo)Gled::TRint_runner_tl, 0, false);
  if (app_thread.Spawn())
  {
    perror(GForm("%scan't create Rint thread", _eh.Data()));
    exit(1);
  }

  gled_exit.Wait();
  gled_exit.Unlock();
  if (gled->GetRintRunning())
  {
    gled->GetRint()->Terminate(0);
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

  gled->StopLogging();
  delete gled;

  GThread::FiniMain();

  exit(0);
}
