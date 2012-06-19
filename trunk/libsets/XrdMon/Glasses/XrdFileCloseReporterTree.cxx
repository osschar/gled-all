// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdFileCloseReporterTree.h"
#include "XrdFileCloseReporterTree.c7"

#include "Glasses/ZLog.h"
#include "Glasses/XrdFile.h"
#include "Glasses/XrdUser.h"
#include "Glasses/XrdServer.h"

#include "Stones/SXrdFileInfo.h"
#include "Stones/SXrdUserInfo.h"
#include "Stones/SXrdServerInfo.h"

#include "Gled/GThread.h"

#include "TFile.h"
#include "TSystem.h"
#include "TTree.h"

// XrdFileCloseReporterTree

//______________________________________________________________________________
//
//

ClassImp(XrdFileCloseReporterTree);

//==============================================================================

void XrdFileCloseReporterTree::_init()
{
  mAutoSaveEntries = 100000;
  mRotateMinutes   = 24 * 60;

  bForceRotate = false;

  mFilePrefix = "xrd-file-access-report-";
  mFile   = 0;
  mTree   = 0;
  mBranchF = mBranchU = mBranchS = 0;
  mXrdF = 0; mXrdU = 0; mXrdS = 0;
}

XrdFileCloseReporterTree::XrdFileCloseReporterTree(const Text_t* n, const Text_t* t) :
  XrdFileCloseReporter(n, t)
{
  _init();
}

XrdFileCloseReporterTree::~XrdFileCloseReporterTree()
{}

//==============================================================================

void XrdFileCloseReporterTree::open_file_create_tree()
{
  static const Exc_t _eh("XrdFileCloseReporterTree::open_file_create_tree ");

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

  mTree = new TTree("XrdFar", "Xrootd File Close Reports");
  mTree->SetAutoFlush(0);
  mTree->SetAutoSave(0);

  mBranchF = mTree->Branch("F.", &mXrdF, 8192);
  mBranchU = mTree->Branch("U.", &mXrdU, 8192);
  mBranchS = mTree->Branch("S.", &mXrdS, 8192);

  mLastFileOpen = GTime::Now();
  bForceRotate = false;
}

void XrdFileCloseReporterTree::write_tree_close_file()
{
  static const Exc_t _eh("XrdFileCloseReporterTree::write_tree_close_file ");

  TString fn(mFile->GetName());

  mTree->Write();
  mFile->Close();
  delete mFile;
  mFile = 0; mTree = 0;
  mBranchF = mBranchU = mBranchS = 0;
  mXrdF = 0; mXrdU = 0; mXrdS = 0;

  if (*mLog)
  {
    mLog->Form(ZLog::L_Message, _eh, "Closed tree file '%s'.", fn.Data());
  }
}

//==============================================================================

void XrdFileCloseReporterTree::ReportLoopInit()
{
  open_file_create_tree();
}

void XrdFileCloseReporterTree::ReportFileClosed(FileUserServer& fus)
{
  static const Exc_t _eh("XrdFileCloseReporterTree::ReportFileClosed ");

  GThread::CancelDisabler _cd;

  if (GTime::ApproximateTime() >= mLastFileOpen + GTime(60*mRotateMinutes, 0) ||
      bForceRotate)
  {
    write_tree_close_file();
    open_file_create_tree();
    {
      GLensReadHolder _lck(this);
      Stamp(FID());
    }
  }

  {
    GLensReadHolder _flck(fus.fFile);
    mXrdF->Assign(fus.fFile);
  }
  {
    GLensReadHolder _ulck(fus.fUser);
    mXrdU->Assign(fus.fUser);
  }
  {
    GLensReadHolder _slck(fus.fServer);
    mXrdS->Assign(fus.fServer);
  }

  mTree->Fill();

  if (mAutoSaveEntries > 0 && mTree->GetEntries() % mAutoSaveEntries == 0)
  {
    if (*mLog)
    {
      mLog->Form(ZLog::L_Info, _eh, "Auto-saving tree, N_entries=%lld.", mTree->GetEntries());
    }
    mTree->AutoSave("FlushBaskets SaveSelf");
  }
}

void XrdFileCloseReporterTree::ReportLoopFinalize()
{
  PARENT_GLASS::ReportLoopFinalize();

  write_tree_close_file();
}

//==============================================================================

void XrdFileCloseReporterTree::RotateTree()
{
  static const Exc_t _eh("XrdFileCloseReporterTree::RotateTree ");

  if ( ! GThread::IsValidPtr(mReporterThread))
    throw _eh + "not running.";

  bForceRotate = true;
}
