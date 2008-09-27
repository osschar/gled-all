// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVDistAnRep_H
#define AliEnViz_AEVDistAnRep_H

#include <Glasses/ZNode.h>

#include "AEVDemoDriver.h"

#include "AEVMlClient.h"
#include "AEVMapViz.h"
#include <Stones/AEVProofFeedback.h>

#include <Glasses/Amphitheatre.h>
#include <Stones/SEvTaskState.h>

#include <Gled/GCondition.h>

class AEVEventBatch;
class AEVJobRep;
class AEVProcessViz;
class AEVDemoDriver;

class AEVDistAnRep : public ZNode
{
  MAC_RNR_FRIENDS(AEVDistAnRep);
  friend class AEVDemoDriver;

private:
  void _init();
  Bool_t	   _first_feedback;

protected:
  typedef list<AEVMlClient::MonaEntry>           lMonaEntry_t;
  typedef list<AEVMlClient::MonaEntry>::iterator lMonaEntry_i;

  ZLink<AEVDemoDriver>     mDemoDriver; //  X{GS} L{}

  ZLink<AList>             mSites;      //  X{GS} L{}
  ZLink<AList>             mEvBatches;  //  X{GS} L{}
  ZLink<AList>             mProcVizes;  //  X{GS} L{} RnrBits{0,0,0,0, 0,0,0,5}

  TString	   mJobId;      // X{GS} 7 Textor(-width=>24, -join=>1)
  TString	   mJobName;    // X{GS} 7 Textor(-width=>8)
  GTime            mQueryTime;  // X{GS}

  Float_t       mConnectTime;	// X{GS} 7 Value(-range=>[0,10,1,100], -join=>1)
  Float_t       mRotTime;	// X{GS} 7 Value(-range=>[0,10,1,100])
  Float_t       mTravelTime;	// X{GS} 7 Value(-range=>[0,10,1,100], -join=>1)
  Float_t       mWaitTime;      // X{GS} 7 Value(-range=>[0,10,1,100])

  lMonaEntry_t     mHistory;        //
  GTime            mProcStart;      // X{GS}
  GTime            mProcEnd;        // X{GS}
  Double_t         mProcDuration;   // X{GS} 7 ValOut(-join=>1)

  Double_t         mReplayDuration; // X{GS} 7 Value(-range=>[0,1000, 1,100], -join=>1)
  Double_t         mReplayMaxWait;  // X{GS} 7 Value(-range=>[0,1000, 1,100])

  Bool_t           bInReplay;       // X{GS} 7 BoolOut(-join=>1)
  Double_t         mReplayPos;      // X{GS} 7 ValOut()
  GTime            mReplayTime;     // X{GS}
  GCondition       mReplayCond;     // X{GS}


  SEvTaskState     mBatchState;	// X{GSR} 7 StoneOutput(Fmt=>"%c: %d [ %d | %d | %d ]", Args=>[State, NAll, NOK, NFail, NProc])
  SEvTaskState     mEventState;	// X{GSR} 7 StoneOutput(Fmt=>"%c: %d [ %d | %d | %d ]", Args=>[State, NAll, NOK, NFail, NProc])

  Int_t  create_evbatches(map<string, int>& cmap);
  void   fix_evbatches();
  Bool_t place_evbatch_on_map(AEVEventBatch* eb);

  AEVProcessViz* find_or_crete_procviz(const TString& proc, const TString& site);

public:
  AEVDistAnRep(const Text_t* n="AEVDistAnRep", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  virtual void AdEnlightenment();
  void AddEventBatch(AEVEventBatch* evb); // X{E} C{1}

  void ResetJob();       // X{E} 7 MButt(-join=>1);
  void ResetDistAn();    // X{E} 7 MButt();

  void InitJob(AEVJobRep* job=0);           // X{ED} C{1} 7 MCWButt()
  void FakeInitJob(AEVJobRep* job=0);       // X{ED} C{1}
  void StudyJobHistory(Bool_t dump_p=true); // X{ED} 7 MCWButt()

  Int_t ReplayJobFromHistory();     // X{ED} 7 MButt(-join=>1)
  Int_t FakeReplayJobFromHistory(); // X{ED}
  void  StopReplay();               // X{E}  7 MButt()
  void  FinalizeJob();		    // X{E}  7 MButt()

  void  FollowJob();            // X{ED} 7 MButt()

  void SendEvBatchesToTheatre(Amphitheatre* amph=0); // X{E} C{1} 7 MButt()


  void Process();       // X{Ed} 7 MButt()
  void UpdateProcStatus(TList* siteinfos, map<string,int>* siteevmap=0);


  void StartProc();     // X{E} 7 MButt()
  void FakeProc();      // X{E} 7 MButt()
  void FakeProc(map<string, int>& changes);

#include "AEVDistAnRep.h7"
  ClassDef(AEVDistAnRep, 1)
}; // endclass AEVDistAnRep


#endif
