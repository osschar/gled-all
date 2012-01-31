// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "UdpPacketTreeWriter.h"
#include "UdpPacketTreeWriter.c7"

// UdpPacketTreeWriter

//______________________________________________________________________________
//
//

ClassImp(UdpPacketTreeWriter);

//==============================================================================

void UdpPacketTreeWriter::_init()
{
  mDFile   = 0;
  mDTree   = 0;
  mDBranch = 0;
}

UdpPacketTreeWriter::UdpPacketTreeWriter(const Text_t* n, const Text_t* t) :
  ZGlass(n, t)
{
  _init();
}

UdpPacketTreeWriter::~UdpPacketTreeWriter()
{}

//==============================================================================

/*
  // Open file, create tree etc frag

  SUdpPacket *pup = 0;

  mDFile   = TFile::Open("xxx.root", "recreate");
  mDTree   = new TTree("Packets", "UDP packets");
  mDTree->SetAutoFlush(-300000);
  mDBranch = mDTree->Branch("P", &pup, 4096, 2);


  // Filler frag

  mDBranch->SetAddress(&pp);

  mDTree->Fill();

  printf("Fill, n=%lld\n", mDTree->GetEntries());

  if (mDTree->GetEntries() % 1000 == 0)
  {
    mDTree->AutoSave("SaveSelf");
  }



void UdpPacketProcessor::StopAllServices()
{
  static const Exc_t _eh("UdpPacketProcessor::StopAllServices ");

  GThread *thr = 0;
  {
    GLensReadHolder _lck(this);
    if ( ! GThread::IsValidPtr(mSuckerThread))
      throw _eh + "not running.";
    thr = mSuckerThread;
    GThread::InvalidatePtr(mSuckerThread);
  }

  thr->Cancel();
  thr->Join();

  printf("Writing tree ...\n");
  mDTree->Write();
  printf("Closing and deleting file ...\n");
  mDFile->Close();
  delete mDFile; mDFile = 0;

  {
    GLensReadHolder _lck(this);
    mSuckerThread = 0;
    mSocket = 0;
  }
}

 */
