// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdFileCloseReporter.h"
#include "XrdFileCloseReporter.c7"

#include "XrdFile.h"

#include <Gled/GThread.h>

// XrdFileCloseReporter

//______________________________________________________________________________
//
//

ClassImp(XrdFileCloseReporter);

//==============================================================================

void XrdFileCloseReporter::_init()
{}

XrdFileCloseReporter::XrdFileCloseReporter(const Text_t* n, const Text_t* t) :
  ZGlass(n, t),
  mReporterThread(0),
  mReporterSocket(0)
{
  _init();
}

XrdFileCloseReporter::~XrdFileCloseReporter()
{}

//==============================================================================

void XrdFileCloseReporter::FileClosed(XrdFile* file)
{
  // put into queue and signal condition

  printf("XrdFileCloseReporter::FileClosed for file=%s\n", file->GetName());

  GMutexHolder _lck(mReporterCond);
  mReporterQueue.push_back(file);
  mReporterCond.Signal();
}

//==============================================================================

void* XrdFileCloseReporter::tl_ReportLoop(XrdFileCloseReporter* r)
{
  r->ReportLoop();
  r->mReporterThread = 0;
  return 0;
}

void XrdFileCloseReporter::ReportLoop()
{
  static const Exc_t _eh("XrdFileCloseReporter::ReportLoop ");

  while (true)
  {
    XrdFile *file = 0;
    {
      GMutexHolder _lck(mReporterCond);
      if (mReporterQueue.empty())
        mReporterCond.Wait();
      file = mReporterQueue.front();
      mReporterQueue.pop_front();
    }
    printf("XrdFileCloseReporter::ReportLoop() now it is I -- %s\n", file->GetName());
  }
}


//==============================================================================

void XrdFileCloseReporter::StartReporter()
{
  static const Exc_t _eh("XrdFileCloseReporter::StartReporter ");

  if (mReporterThread)
    throw _eh + "already running.";

  mReporterThread = new GThread("XrdFileCloseReporter-Reporter",
			      (GThread_foo) tl_ReportLoop, this,
			      false);
  mReporterThread->SetNice(20);
  mReporterThread->Spawn();
}

void XrdFileCloseReporter::StopReporter()
{
  static const Exc_t _eh("XrdFileCloseReporter::StopReporter ");

  if (mReporterThread == 0)
    throw _eh + "not running.";

  mReporterThread->Cancel();
  mReporterThread->Join();
  mReporterThread = 0;
  mReporterQueue.clear();
  // mReporterSocket ???
}
