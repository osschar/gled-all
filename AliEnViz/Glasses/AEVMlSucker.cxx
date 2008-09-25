// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AEVMlSucker.h"
#include "AEVMlSucker.c7"

#include <Gled/GThread.h>

#include <TSystem.h>

// AEVMlSucker

//__________________________________________________________________________
//
//

ClassImp(AEVMlSucker);

/**************************************************************************/

void AEVMlSucker::_init()
{
  mSuckerThread = 0;
}

/**************************************************************************/

void* AEVMlSucker::tl_Suck(AEVMlSucker* s)
{
  s->Suck();
  s->mSuckerThread = 0;
  return 0;
}

void AEVMlSucker::Suck()
{
  static const Exc_t _eh("AEVMlSucker::Suck ");

  static const TString cmd("nc pcalimonitor2.cern.ch 7014");

  FILE* s = gSystem->OpenPipe(cmd, "r");
  if (s == 0)
  {
    perror(GForm("PipeOpen of '%s' failed:", cmd.Data()));
    return;
  }

  char buff[1024];
  while (true)
  {
    if (fgets(buff, 1024, s) == 0)
    {
      perror("Error sucking:");
      break;
    }
    int len = strlen(buff);
    // chomp
    if (len && buff[len-1] == 10) buff[len-1] = 0;

    printf("Sucked: %s\n", buff);

    TString l(buff);
    
  }
}

void AEVMlSucker::StartSucker()
{
  static const Exc_t _eh("AEVMlSucker::StartSucker ");

  if (mSuckerThread)
    throw (_eh + "already running.");


  mSuckerThread = new GThread("AEVMlSucker-Sucker",
			      (GThread_foo) tl_Suck, this,
			      false);
  mSuckerThread->SetNice(10);
  mSuckerThread->Spawn();
}

void AEVMlSucker::StopSucker()
{
  static const Exc_t _eh("AEVMlSucker::StopSucker ");

  if (mSuckerThread == 0)
    throw (_eh + "not running.");

  mSuckerThread->Cancel();
  mSuckerThread->Join();
  mSuckerThread = 0;
}

/**************************************************************************/
