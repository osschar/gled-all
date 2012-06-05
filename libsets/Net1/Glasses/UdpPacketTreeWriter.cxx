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

// UdpPacketTreeWriter

//______________________________________________________________________________
//
//

ClassImp(UdpPacketTreeWriter);

//==============================================================================

void UdpPacketTreeWriter::_init()
{
  mWLThread = 0;

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
  TString fn;
  TString today = GTime::Now().ToDateLocal();
  Int_t   i = 0;
  while (true)
  {
    fn = mFilePrefix + today;
    if (i != 0)
      fn += TString::Format("-%d", i);
    fn += ".root";

    // Does it exist?

    ++i;
  }

  // Open sesame
}

void UdpPacketTreeWriter::write_tree_close_file()
{

}

//==============================================================================

void UdpPacketTreeWriter::Start()
{
  // Open file, create tree etc frag

  static const Exc_t _eh("UdpPacketTreeWriter::Start ");

  {
    GLensReadHolder _lck(this);
    if (mWLThread != 0)
      throw _eh + "already running.";
    mWLThread = GThread::Self();
  }

  SUdpPacket *pup = 0;

  mFile   = TFile::Open("xxx.root", "recreate");
  mTree   = new TTree("Packets", "UDP packets");
  mTree->SetAutoFlush(-300000);
  mBranch = mTree->Branch("P", &pup, 4096, 2);

  mSource->RegisterConsumer(&mUdpQueue);

  // Filler frag
  while (true)
  {
    SUdpPacket *p = mUdpQueue.PopFront();

    mBranch->SetAddress(&p);

    mTree->Fill();

    printf("Fill, n=%lld\n", mTree->GetEntries());

    if (mTree->GetEntries() % 1000 == 0)
    {
      mTree->AutoSave("SaveSelf");
    }

    p->DecRefCount();
  }
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

  mSource->UnregisterConsumer(&mUdpQueue);

  thr->Cancel();
  thr->Join();

  printf("Writing tree ...\n");
  mTree->Write();
  printf("Closing and deleting file ...\n");
  mFile->Close();
  delete mFile; mFile = 0;

}

//==============================================================================
