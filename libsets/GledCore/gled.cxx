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

#include <Getline.h>
#include <TROOT.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifndef __APPLE__
#include <X11/Xlib.h>
#endif

/**************************************************************************/

int main(int argc, char **argv)
{
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

  GledGUI* gled = new GledGUI();
  gled->ReadArguments(argc, argv);
  gled->ParseArguments(false);

  if (gled->GetQuit())
  {
    exit(0);
  }

  GThread::InitMain();

  gled->InitLogging();
  gled->InitGledCore();

  // Run Root Application thread
  GThread *app_thread = gled->SpawnRootAppThread("gled.cxx");

  // Run Gled ... FLTK event loop for GledGUI. [ This is somewhat silly. ]
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

  // Mac only.
  // Seems we need to stop all threads that are still alive.
  // Not that any should be ...
#ifdef __APPLE__
  GThread::CancelAllThreads(true);
#endif

  gled->StopLogging();
  delete gled;

  GThread::FiniMain();

  // Mac only.
  // Consistent crash in:
  // #0  0x0000000100daf2db in TObject::TestBit ()
  // #1  0x000000010238c6a1 in TObject::IsOnHeap (this=0x10d80fb90) at TObject.h:138
  // #2  0x000000010244af5f in TList::Delete (this=0x104613a40, option=0x1029263f0 "") at TList.cxx:412
  // #3  0x00000001023bfeac in TROOT::~TROOT (this=0x104613a40) at TROOT.cxx:490
  // #4  0x00007fff8f92d7c8 in __cxa_finalize ()
  // #5  0x00007fff8f92d652 in exit ()
  // #6  0x000000010000145e in main ()
  // Here, all seems well.
#ifdef __APPLE__
  if (true)
  {
    TSeqCollection *s = gROOT->GetListOfSpecials();
    printf("About to ditch gROOT, now is %p, is-valid %d; s=%p, s_on_heap=%d\n",
	   gROOT, gROOT->TestBit(TObject::kInvalidObject),
	   s, s->IsOnHeap());
  }
  // So, let's trick ~TROOT into not doing anything.
  gROOT = 0;
#endif

  exit(Gled::GetExitStatus());
}
