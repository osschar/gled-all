// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "UdpPacketTreeWriter.h"
#include "Glasses/UdpPacketSource.h"
#include "Glasses/ZLog.h"
#include "UdpPacketTreeWriter.c7"

#include "Stones/SUdpPacket.h"
#include "Gled/GThread.h"

#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"

// UdpPacketTreeWriter

//______________________________________________________________________________
//
//

ClassImp(UdpPacketTreeWriter);

//==============================================================================

void UdpPacketTreeWriter::_init()
{
  mWLThread = 0;

  mAutoSaveEntries = 100000;
  mRotateMinutes   = 24 * 60;

  bRunning = false;
  bForceRotate = false;

  mFilePrefix = "udp-tree-dump-";
  mFile   = 0;
  mTree   = 0;
  mBranch = 0;
}

UdpPacketTreeWriter::UdpPacketTreeWriter(const Text_t* n, const Text_t* t) :
  ZGlass(n, t)
{
  _init();
}

UdpPacketTreeWriter::~UdpPacketTreeWriter()
{}

//==============================================================================

void UdpPacketTreeWriter::open_file_create_tree()
{
  static const Exc_t _eh("UdpPacketTreeWriter::open_file_create_tree ");

  TString fn;
  TString basename = mFilePrefix + GTime::Now().ToDateLocal();
  Int_t   i = 0;
  while (true)
  {
    fn = basename;
    if (i != 0)
      fn += TString::Format("-%d", i);
    fn += ".root";

    if (gSystem->AccessPathName(fn) == true)
    {
      // No file with this name yet ... use it.
      break;
    }

    ++i;
  }

  if (*mLog)
  {
    mLog->Form(ZLog::L_Message, _eh, "Opening tree file '%s'.", fn.Data());
  }

  mFile = TFile::Open(fn, "recreate");

  mTree = new TTree("Packets", "UDP packets");
  mTree->SetAutoFlush(0);
  mTree->SetAutoSave(0);

  SUdpPacket *pup = 0;
  mBranch = mTree->Branch("P", &pup, 8192, 2);

  mLastFileOpen = GTime::Now();
  bForceRotate = false;
}

void UdpPacketTreeWriter::write_tree_close_file()
{
  static const Exc_t _eh("UdpPacketTreeWriter::write_tree_close_file ");

  TString fn(mFile->GetName());

  mTree->Write();
  mFile->Close();
  delete mFile;
  mFile = 0; mTree = 0; mBranch = 0;

  if (*mLog)
  {
    mLog->Form(ZLog::L_Message, _eh, "Closed tree file '%s'.", fn.Data());
  }
}

//==============================================================================

void UdpPacketTreeWriter::cu_WriteLoop(UdpPacketTreeWriter* w)
{
  w->mSource->UnregisterConsumer(&w->mUdpQueue);
  w->mUdpQueue.ClearQueueDecRefCount();

  w->write_tree_close_file();

  {
    GLensReadHolder _lck(w);
    w->mWLThread = 0;
    w->bRunning = false;
    w->Stamp(w->FID());
  }
}

void UdpPacketTreeWriter::WriteLoop()
{
  static const Exc_t _eh("UdpPacketTreeWriter::WriteLoop ");

  while (true)
  {
    SUdpPacket *p;

    {
      GThread::CancelEnabler _ce;
      p = mUdpQueue.PopFrontTimedWaitUntil(GTime::ApproximateNow() + GTime(10, 0));
    }

    if (GTime::ApproximateNow() >= mLastFileOpen + GTime(60*mRotateMinutes, 0) || bForceRotate)
    {
      mBranch->SetAddress(0);
      write_tree_close_file();
      open_file_create_tree();
      {
        GLensReadHolder _lck(this);
        Stamp(FID());
      }
    }

    if (p == 0)
      continue;

    mBranch->SetAddress(&p);
    mTree->Fill();

    if (mAutoSaveEntries > 0 && mTree->GetEntries() % mAutoSaveEntries == 0)
    {
      if (*mLog)
      {
        mLog->Form(ZLog::L_Info, _eh, "Auto-saving tree, N_entries=%lld.", mTree->GetEntries());
      }
      mTree->AutoSave("FlushBaskets SaveSelf");
    }

    p->DecRefCount();
  }
}

//==============================================================================

void UdpPacketTreeWriter::Start()
{
  // Open file, create tree etc frag

  static const Exc_t _eh("UdpPacketTreeWriter::Start ");

  {
    GLensReadHolder _lck(this);
    if (mWLThread)
      throw _eh + "already running.";

    mWLThread = GThread::Self();
    mWLThread->SetName("UdpPacketTreeWriter-WriteLoop");
    GThread::CancelOff();

    bRunning = true;
    Stamp(FID());
  }

  open_file_create_tree();

  mWLThread->CleanupPush((GThread_cu_foo) cu_WriteLoop, this);

  mSource->RegisterConsumer(&mUdpQueue);

  WriteLoop();

  mWLThread->CleanupPop(true);
}

void UdpPacketTreeWriter::Stop()
{
  static const Exc_t _eh("UdpPacketTreeWriter::Stop ");

  GThread *thr = 0;
  {
    GLensReadHolder _lck(this);
    if ( ! GThread::IsValidPtr(mWLThread))
      throw _eh + "not running.";
    thr = mWLThread;
    GThread::InvalidatePtr(mWLThread);
  }

  thr->Cancel();
  thr->Join();

  delete thr;
}

//==============================================================================

void UdpPacketTreeWriter::RotateTree()
{
  static const Exc_t _eh("UdpPacketTreeWriter::RotateTree ");

  if ( ! GThread::IsValidPtr(mWLThread))
    throw _eh + "not running.";

  bForceRotate = true;
}
