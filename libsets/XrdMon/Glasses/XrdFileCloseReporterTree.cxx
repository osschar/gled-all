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

#include "TCint.h"
#include "TVirtualMutex.h"

// XrdFileCloseReporterTree

//______________________________________________________________________________
//
// * bFileIdxAlways: Always posfix file name with an index.

ClassImp(XrdFileCloseReporterTree);

//==============================================================================

void XrdFileCloseReporterTree::_init()
{
  mAutoSaveEntries  = 100000;
  mAutoSaveMinutes  = 60;
  mRotateMinutes    = 24 * 60;
  bRotateAtMidnight = true;

  bForceAutoSave = bForceRotate = false;

  bFileIdxAlways = true;
  mFilePrefix    = "xrd-file-access-report-";
  mTreeName      = "XrdFar";
  mFileLastIdx   = -1;
  mFile = 0;
  mTree = 0;
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

  TString date     = GTime::Now().ToDateLocal();
  TString basename = mFilePrefix + date;

  if (date == mFileLastDate)
  {
    ++mFileLastIdx;
    mFileNameTrue = basename + TString::Format("-%d.root", mFileLastIdx);
  }
  else
  {
    if (mFileLastDate.IsNull())
    {
      // We are just starting ... check if previous files exist.
      Int_t i = 0;
      while (true)
      {
        mFileNameTrue = basename;
        if (bFileIdxAlways || i != 0) mFileNameTrue += TString::Format("-%d", i);
        mFileNameTrue += ".root";

        if (gSystem->AccessPathName(mFileNameTrue) == true)
        {
          // No file with this name yet ... check also if a hidden file
	  // exists from a previous run (could happen if it crashed).
	  TString hfn = GledNS::pathname_make_hidden_file(mFileNameTrue);
	  if (gSystem->AccessPathName(hfn) == true)
	  {
	    break;
	  }
          else if (*mLog)
          {
            mLog->Form(ZLog::L_Warning, _eh, "Hidden file '%s' found during start-up - it will be kept as is.", hfn.Data());
          }
        }

        ++i;
      }
      mFileLastIdx = i;
    }
    else
    {
      mFileNameTrue = basename;
      if (bFileIdxAlways) mFileNameTrue += "-0";
      mFileNameTrue += ".root";

      mFileLastIdx = 0;
    }
    mFileLastDate = date;
  }

  if (*mLog)
  {
    mLog->Form(ZLog::L_Message, _eh, "Opening tree file '%s' (kept hidden until closing).", mFileNameTrue.Data());
  }

  TString fn = GledNS::pathname_make_hidden_file(mFileNameTrue);
  {
    R__LOCKGUARD2(gCINTMutex);

    mFile = TFile::Open(fn, "recreate");
    if (mFile == 0)
    {
      throw _eh + "Opening of file '" + fn + "' failed.";
    }

    mTree = new TTree(mTreeName, "Xrootd File Close Reports");
    mTree->SetAutoFlush(1000);
    mTree->SetAutoSave(0);

    mBranchF = mTree->Branch("F.", &mXrdF);
    mBranchU = mTree->Branch("U.", &mXrdU);
    mBranchS = mTree->Branch("S.", &mXrdS);
  }

  mLastAutoSave  = GTime::ApproximateTime();
  bForceAutoSave = false;

  mLastFileOpen  = GTime::ApproximateTime();
  bForceRotate   = false;
}

void XrdFileCloseReporterTree::write_tree_close_file()
{
  static const Exc_t _eh("XrdFileCloseReporterTree::write_tree_close_file ");

  TString fn(mFile->GetName());

  mTree->Write();

  TNamed xx("WritingComplete", "");
  mFile->WriteTObject(&xx);

  mFile->Close();
  delete mFile;
  mFile = 0; mTree = 0;
  mBranchF = mBranchU = mBranchS = 0;
  mXrdF = 0; mXrdU = 0; mXrdS = 0;

  gSystem->Rename(fn, mFileNameTrue);

  if (*mLog)
  {
    mLog->Form(ZLog::L_Message, _eh, "Closed tree file '%s'.", mFileNameTrue.Data());
  }

  mFileNameTrue = "";
}

//------------------------------------------------------------------------------

void XrdFileCloseReporterTree::check_file_rotate()
{
  // Should be called with cancellation disabled, no lock.

  GTime at = GTime::ApproximateTime();
  if ((mRotateMinutes > 0 && at >= mLastFileOpen + GTime(60*mRotateMinutes, 0)) ||
      (bRotateAtMidnight && at - mLastFileOpen > at.TimeOfTheDayLocal()) ||
      bForceRotate)
  {
    write_tree_close_file();
    open_file_create_tree();
    {
      GLensReadHolder _lck(this);
      Stamp(FID());
    }
  }
}

//==============================================================================

void XrdFileCloseReporterTree::ReportLoopInit()
{
  mFileNameTrue = "";
  mFileLastDate = "";
  mFileLastIdx  = -1;

  open_file_create_tree();
}

void XrdFileCloseReporterTree::ReportFileClosed(FileUserServer& fus)
{
  static const Exc_t _eh("XrdFileCloseReporterTree::ReportFileClosed ");

  check_file_rotate();

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

  if ((mAutoSaveEntries > 0 && mTree->GetEntries() % mAutoSaveEntries == 0) ||
      (mAutoSaveMinutes > 0 && GTime::ApproximateTime() >= mLastAutoSave + GTime(60*mAutoSaveMinutes, 0)) ||
      bForceAutoSave)
  {
    if (*mLog)
    {
      mLog->Form(ZLog::L_Info, _eh, "Auto-saving tree, N_entries=%lld.", mTree->GetEntries());
    }
    mTree->AutoSave("SaveSelf");

    {
      GLensReadHolder _lck(this);
      mLastAutoSave  = GTime::ApproximateTime();
      bForceAutoSave = false;
      Stamp(FID());
    }
  }
}

void XrdFileCloseReporterTree::ReportCondWaitTimeout()
{
  check_file_rotate();
}

void XrdFileCloseReporterTree::ReportLoopFinalize()
{
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

void XrdFileCloseReporterTree::AutoSaveTree()
{
  static const Exc_t _eh("XrdFileCloseReporterTree::AutoSaveTree ");

  if ( ! GThread::IsValidPtr(mReporterThread))
    throw _eh + "not running.";

  bForceAutoSave = true;
}
