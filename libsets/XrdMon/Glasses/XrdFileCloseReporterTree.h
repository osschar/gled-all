// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef XrdMon_XrdFileCloseReporterTree_H
#define XrdMon_XrdFileCloseReporterTree_H

#include <Glasses/XrdFileCloseReporter.h>

class ZLog;

class SXrdFileInfo;
class SXrdUserInfo;
class SXrdServerInfo;
class SXrdIoInfo;

class TFile;
class TTree;
class TBranch;


class XrdFileCloseReporterTree : public XrdFileCloseReporter
{
  MAC_RNR_FRIENDS(XrdFileCloseReporterTree);

private:
  void _init();

protected:
  Int_t                   mAutoSaveEntries;  // X{GS} 7 Value(-range=>[0, 1000000, 1])
  Int_t                   mAutoSaveMinutes;  // X{GS} 7 Value(-range=>[0, 14400, 1])
  GTime                   mLastAutoSave;     //!X{GR} 7 TimeOut()
  Int_t                   mRotateMinutes;    // X{GS} 7 Value(-range=>[0, 14400, 1])
  Bool_t                  bRotateAtMidnight; // X{GS} 7 Bool()
  GTime                   mLastFileOpen;     //!X{GR} 7 TimeOut()

  Bool_t                  bForceAutoSave;    //!
  Bool_t                  bForceRotate;      //!

  Bool_t                  bStoreIoInfo;      // X{GS} 7 Bool()
  Bool_t                  bFileIdxAlways;    // X{GS} 7 Bool()
  TString                 mFilePrefix;       // X{GS} 7 Textor()
  TString                 mTreeName;         // X{GS} 7 Textor()
  TString                 mFileNameTrue;     //!
  TString                 mFileLastDate;     //!
  Int_t                   mFileLastIdx;      //!
  TFile                  *mFile;             //!
  TTree                  *mTree;             //!
  TBranch                *mBranchF;          //!
  TBranch                *mBranchU;          //!
  TBranch                *mBranchS;          //!
  TBranch                *mBranchI;          //!

  SXrdFileInfo           *mXrdF;             //!
  SXrdUserInfo           *mXrdU;             //!
  SXrdServerInfo         *mXrdS;             //!
  SXrdIoInfo             *mXrdI;             //!

  void open_file_create_tree();
  void write_tree_close_file();

  void check_file_rotate();

  virtual void ReportLoopInit();
  virtual void ReportFileClosed(FileUserServer& fus);
  virtual void ReportCondWaitTimeout();
  virtual void ReportLoopFinalize();

public:
  XrdFileCloseReporterTree(const Text_t* n="XrdFileCloseReporterTree", const Text_t* t=0);
  virtual ~XrdFileCloseReporterTree();

  void RotateTree();   // X{E}  7 MButt()
  void AutoSaveTree(); // X{E}  7 MButt()

#include "XrdFileCloseReporterTree.h7"
  ClassDef(XrdFileCloseReporterTree, 1);
}; // endclass XrdFileCloseReporterTree

#endif
