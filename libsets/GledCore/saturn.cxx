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

#include <Getline.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

/**************************************************************************/

int main(int argc, char **argv)
{
  Gled* gled = new Gled();
  gled->ReadArguments(argc, argv);
  gled->ParseArguments(true);

  if (gled->GetQuit())
  {
    exit(0);
  }

  if (gled->GetDaemon())
  {
    printf("saturn - daemonizing ...\n");
    if (daemon(1, 1))
    {
      perror("saturn - daemonization failed");
      exit(1);
    }
  }

  GThread::InitMain();

  gled->InitLogging();
  gled->InitGledCore();

  GCondition gled_exit;
  gled_exit.Lock();
  gled->SetExitCondVar(&gled_exit);

  // Run Root Application thread
  GThread *app_thread = gled->SpawnRootAppThread("saturn.cxx");

  gled->Run();

  if (gled->GetRootAppRunning())
  {
    app_thread->Kill(GThread::SigTERM);
  }
  else
  {
    Getlinem(kCleanUp, 0);
  }
  app_thread->Join();

  gled->StopLogging();
  delete gled;

  GThread::FiniMain();

  exit(Gled::GetExitStatus());
}
