// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AEVDemoDriver
//
//

#include "AEVDemoDriver.h"

#include "AEVMlClient.h"
#include "AEVMapViz.h"
#include "AEVDistAnRep.h"
#include "AEVJobRep.h"

#include <Glasses/Amphitheatre.h>
#include <Glasses/WSTube.h>

#include <Glasses/Eventor.h>
#include <Glasses/TimeMaker.h>
#include <Glasses/WGlDirectory.h>
#include <Glasses/WGlFrameStyle.h>
#include <Glasses/WGlButton.h>

#include "AEVDemoDriver.c7"

#include "AEVFlatSSpace.h"
#include "AEVEventBatch.h"
#include "AEVSiteViz.h"

#include <Glasses/ZQueen.h>

#include <TMath.h>
#include <TSystem.h>

ClassImp(AEVDemoDriver)

/**************************************************************************/

void AEVDemoDriver::_init()
{
  mText = "ALICE Grid Demo";

  mConnectTime = 4;
  mRotTime     = 2;
  mTravelTime  = 2;
  mWaitTime    = 0.02;

  mFPS     = 20;
  mRPS     = 20;

  mRnd.SetSeed(0);

  bWorking = false;

  bFakeProcessing = false;
}

bool AEVDemoDriver::trylock()
{
  if(bWorking) {
    printf("demodriver ... lock refused\n");
    return false;
  }
  disable_menu();
  bWorking = true;
  return true;
}

void AEVDemoDriver::unlock()
{
  enable_menu();
  bWorking = false;
}

void AEVDemoDriver::enable_menu()
{
  mMenuFrameStyle->SetTileColor(0.3, 0.3, 0.5, 0.6);
  mMenuFrameStyle->SetBelowMColor(0.5, 0.5, 0.7, 0.8);
}

void AEVDemoDriver::disable_menu()
{
  mMenuFrameStyle->SetTileColor(0.5, 0.3, 0.3, 0.4);
  mMenuFrameStyle->SetBelowMColor(0.7, 0.5, 0.5, 0.6);
}

/**************************************************************************/

void AEVDemoDriver::InitAnimationStuff(Eventor* e, TimeMaker* tm)
{
  // Configuration helper.

  if(e == 0) {
    e = new Eventor("Animator");
    mQueen->CheckIn(e);
  }
  SetAnimator(e);
  Add(e);
  if(tm == 0) {
    tm = new TimeMaker("Animation TimeMaker");
    mQueen->CheckIn(tm);
  }
  SetAnimTimer(tm);
  mAnimator->Add(tm);

  mAnimator->SetInterBeatMS(TMath::Nint(1000.0/mRPS));
  mAnimator->Start();
}

/**************************************************************************/

inline Double_t AEVDemoDriver::rnd(Double_t k, Double_t n)
{
  return k*mRnd.Rndm() + n;
}

/**************************************************************************/

void AEVDemoDriver::sleep(Float_t s)
{
  GTime::SleepMiliSec(UInt_t(1000*s*mFPS/mRPS));
}

void AEVDemoDriver::l_sleep(Float_t s)
{
  WriteUnlock();
  sleep(s);
  WriteLock();
}

void AEVDemoDriver::set_text(const Text_t* txt)
{
  WriteLock();
  SetText(txt);
  WriteUnlock();
  mInfoBar->SetText(txt);
}

void AEVDemoDriver::set_blurp(const Text_t* txt)
{
  mBlurpBar->SetText(txt);
}
/**************************************************************************/

void AEVDemoDriver::setup_tube(WSTube* tube, const Text_t* texname)
{
  tube->SetFat(false);
  tube->SetLineW(1);
  tube->SetTexture((ZImage*) mTexCont->GetElementByName(texname));
  tube->SetTLevel(30);
  tube->SetPLevel(3);
  tube->SetDefWidth(0.04);
  tube->SetDefTension(1.5);

  tube->SetVecA(TLorentzVector(0,     0,   1,   1));
  tube->SetSgmA(TLorentzVector(0.2, 0.2, 0.1, 0.1));
  tube->SetVecB(TLorentzVector(0,     0,  -1,   1));
  tube->SetSgmB(TLorentzVector(0.2, 0.2, 0.1, 0.1));
}

void AEVDemoDriver::animate_tube(WSTube* tube)
{
  mAnimTimer->AddClient(tube);
}

void AEVDemoDriver::remove_tube(WSTube* tube)
{
  if(tube != 0)
    mQueen->RemoveLens(tube);
}

/**************************************************************************/

void AEVDemoDriver::rotate(ZNode* node, Float_t time, Int_t ai, Int_t aj)
{
  int nmax = int(time*mFPS);
  for(int i=0; i<nmax; ++i) {
    node->WriteLock();
    node->RotateLF(ai, aj, 0.025);
    node->WriteUnlock();
    gSystem->Sleep(TMath::Nint(1000.0/mRPS));
  }
}

/**************************************************************************/

void AEVDemoDriver::ResetDemo()
{
  {
    GLensWriteHolder wlk(*mDistAnRep);
    mDistAnRep->ResetDistAn();
  }
  set_text("ALICE Grid Demo");
}

void AEVDemoDriver::RunDemo()
{
  Float_t ss = 2.5;

  ConnectMonaLisa();
  sleep(ss);

  QueryJobs();
  sleep(ss);

  QueryJobDetails(dynamic_cast<AEVJobRep*>(mMonaClient->FrontElement()));
  sleep(ss);

  ReplayJob(0);

  // ConnectToProof();     sleep(ss);
  // ConnectProofSlaves(); sleep(ss);
  // ProcessEvents();
}

/**************************************************************************/

void AEVDemoDriver::ConnectMonaLisa()
{
  static const Exc_t _eh("AEVDemoDriver::ConnectMonaLisa ");

  if(mMonaTube != 0)
    throw(_eh + "already connected.");

  if(trylock() == false) return;

  set_text("Connecting to MonaLisa ...");

  WSTube* mona_tube = new WSTube("MonaTube");
  mQueen->CheckIn(mona_tube);
  { GLensWriteHolder monalck(mona_tube);
    mona_tube->SetRnrMod(mQueen->FindLensByPath("lib/rnrmods/BackCuller"));
    mona_tube->SetTexture((ZImage*) mTexCont->GetElementByName("Red2Cyan"));
    mona_tube->SetTLevel(50);
    mona_tube->SetPLevel(8);
    mona_tube->SetFat(true);
    mona_tube->SetColor(1, 1, 1, 0.5);
    mona_tube->SetDtexU(-0.2);
    mona_tube->SetDefWidth(0.04);
    mona_tube->SetDefTension(3.2);
    mona_tube->SetNodeA(mGledViz->GetParent());
    mona_tube->SetNodeB(mMonaViz->GetParent());
    mona_tube->SetVecA(TLorentzVector(1, -2,  1, 1));
    mona_tube->SetVecB(TLorentzVector(1,  2, -1, 1));
    mona_tube->SetDefVelocity(1.0 / mTravelTime);
    mona_tube->SetMinWaitTime(mWaitTime);
  }
  { GLensWriteHolder wrlck(this);
    Add(mona_tube);
    SetMonaTube(mona_tube);
    mAnimTimer->AddClient(mona_tube);
  }
  mMonaTube->AnimateConnect(1 / mConnectTime);
  sleep(mConnectTime);
  if(!bFakeProcessing)
    mMonaClient->check_connection(_eh);

  set_text("Logging in ...");
  { GLensWriteHolder monalck(*mMonaTube);
    mMonaTube->TravelAtoB();
  }
  sleep(0.3*mTravelTime);
  { GLensWriteHolder monalck(*mMonaTube);
    mMonaTube->TravelBtoA();
  }
  sleep(1.2*mTravelTime);

  set_text("Connected to MonALISA.");

  unlock();
}

void AEVDemoDriver::DisconnectMonaLisa()
{
  static const Exc_t _eh("AEVDemoDriver::DisconnectMonaLisa ");

  if(mMonaTube == 0)
    throw(_eh + "not connected.");

  if(trylock() == false) return;

  set_text("Disconnecting from MonaLisa ...");

  { GLensWriteHolder monalck(*mMonaTube);
    mMonaTube->AnimateDisconnect(1 / mConnectTime);
  }
  if(!bFakeProcessing)
    mMonaClient->CloseConn();
  sleep(mConnectTime);

  remove_tube(*mMonaTube);

  set_text("MonALISA disconnected.");

  unlock();
}

/**************************************************************************/

void AEVDemoDriver::QueryJobs()
{
  static const Exc_t _eh("AEVDemoDriver::QueryJobs ");
  set_blurp("");

  if(mMonaTube == 0) throw(_eh + "MonALISA not connected.");

  if(trylock() == false) return;

  Float_t ttravel = 2.5;

  set_text("Querying jobs ...");

  { GLensWriteHolder wrlck(*mMonaTube);
    mMonaTube->TravelAtoB();
  }
  sleep(ttravel);
  rotate(*mMonaViz, mRotTime, 1, 2);

  if(bFakeProcessing == false)
  {
    auto_ptr<ZMIR> jq( mMonaClient->S_FindJobs() );
    mSaturn->ShootMIRWaitResult(jq);
    int ntravel = TMath::Min(mMonaClient->Size(), 20);
    { GLensWriteHolder wrlck(*mMonaTube);
      for(int i=0; i<ntravel; ++i) {
	mMonaTube->TravelBtoA();
      }
    }
    sleep(ttravel);
    rotate(*mGledViz, ntravel*mWaitTime, 1, 2);
  }
  else
  {
    auto_ptr<ZMIR> jq( mMonaClient->S_FakeFindJobs() );
    mSaturn->ShootMIRWaitResult(jq);
    int ntravel = TMath::Min(mMonaClient->Size(), 20);
    { GLensWriteHolder wrlck(*mMonaTube);
      for(int i=0; i<ntravel; ++i) {
	mMonaTube->TravelBtoA();
      }
    }
    sleep(ttravel);
    rotate(*mGledViz, ntravel*mWaitTime, 1, 2);
  }
  mJobDirectory->SetRnrSelf(true);

  set_text("Jobs imported.");

  unlock();
}

void AEVDemoDriver::QueryJobDetails(AEVJobRep* job_rep)
{
  static const Exc_t _eh("AEVDemoDriver::QueryJobDetails ");

  if(mMonaTube == 0) throw(_eh + "MonALISA not connected.");
  if(job_rep   == 0) throw(_eh + "job_rep argument null.");

  if(trylock() == false) return;

  set_text("Querying job details ...");
  set_blurp("");

  {
    GLensWriteHolder wrlck(*mMonaTube);
    mMonaTube->MakeTraveler(2/mTravelTime);
  }

  if(bFakeProcessing == false)
  {
    auto_ptr<ZMIR> jq( mDistAnRep->S_InitJob(job_rep) );
    mSaturn->ShootMIRWaitResult(jq);
  }
  else
  {
    auto_ptr<ZMIR> jq( mDistAnRep->S_FakeInitJob(job_rep) );
    mSaturn->ShootMIRWaitResult(jq);
  }

  set_text("Importing job info ...");

  int ntravel = TMath::Min(mDistAnRep->GetSites()->Size(), 5);
  {
    GLensWriteHolder wrlck(*mMonaTube);
    for(int i=0; i<ntravel; ++i)
    {
      mMonaTube->MakeTraveler(-2/mTravelTime);
    }
  }

  set_text("Job details received.");

  unlock();
}

/**************************************************************************/

void AEVDemoDriver::ReplayJob(WGlButton* button)
{
  static const Exc_t _eh("AEVDemoDriver::ReplayJob ");

  if(mMonaTube == 0) throw(_eh + "MonALISA not connected.");

  if(mDistAnRep->mJobName == "") throw(_eh + "no job selected.");

  if(trylock() == false) return;

  set_text("Querying job history ...");

  { GLensWriteHolder wrlck(*mMonaTube);
    mMonaTube->MakeTraveler(2.5/mTravelTime);
  }
  { GLensWriteHolder wrlck(*mDistAnRep);
    mDistAnRep->ResetJob();
  }
  if(bFakeProcessing == false) {
    auto_ptr<ZMIR> jq( mDistAnRep->S_StudyJobHistory() );
    mSaturn->ShootMIRWaitResult(jq);
  }

  set_text("Importing job history ...");

  mJobDirectory->SetRnrSelf(false);

  int ntravel = TMath::Min(mDistAnRep->mHistory.size()/60ul, 20ul);
  { GLensWriteHolder wrlck(*mMonaTube);
    for(int i=0; i<ntravel; ++i) {
      mMonaTube->MakeTraveler(-2.5/mTravelTime);
    }
  }

  if(button) {
    button->MenuEnter();
  }

  { GLensWriteHolder wrlck(*mDistAnRep);
    mDistAnRep->SendEvBatchesToTheatre();
  }

  unlock();

  DoJobReplay();
}

void AEVDemoDriver::CycleJob(WGlButton* button)
{
  static const Exc_t _eh("AEVDemoDriver::CycleJob ");

  bCycling = true;
  while(bCycling) {
    ReplayJob(button);
    button = 0; // Only do menu enter the first time.
    sleep(5);
  }
}

void AEVDemoDriver::DoJobReplay()
{
  if(trylock() == false) return;

  set_text("Replaying job ...");

  enable_menu();

  if(bFakeProcessing == false) {
    auto_ptr<ZMIR>     jq( mDistAnRep->S_ReplayJobFromHistory() );
    auto_ptr<ZMIR_RR> res( mSaturn->ShootMIRWaitResult(jq) );
    if(res->HasResult()) {
      Int_t nleft; *res >> nleft;
      if(nleft > 0)
	set_text("Job replay paused.");
      else
	set_text("Job replay done.");
    } else if(res->HasException()) {
      set_text("Job replay aborted.");
      set_blurp(res->fException);
    }
  } else {
    auto_ptr<ZMIR>     jq( mDistAnRep->S_FakeReplayJobFromHistory() );
    auto_ptr<ZMIR_RR> res( mSaturn->ShootMIRWaitResult(jq) );
    if(res->HasResult()) {
      Int_t nleft; *res >> nleft;
      if(nleft > 0)
	set_text("Job replay paused.");
      else
	set_text("Job replay done.");
    } else if(res->HasException()) {
      set_text("Job replay aborted.");
      set_blurp(res->fException);
    }
  }

  unlock();
}

void AEVDemoDriver::PauseJobReplay()
{
  { GLensWriteHolder wrlck(*mDistAnRep);
    if(mDistAnRep->GetInReplay())
      mDistAnRep->StopReplay();
  }
}

void AEVDemoDriver::FinalizeJob()
{
  if(trylock() == false) return;

  set_text("Finalizing job ...");

  { GLensWriteHolder wrlck(*mDistAnRep);
    mDistAnRep->FinalizeJob();
  }

  set_text("Job finalized.");

  unlock();
}

void AEVDemoDriver::ExitReplayMenu(WGlButton* button)
{
  if(mDistAnRep->GetInReplay())
    mDistAnRep->StopReplay();

  bCycling = false;

  set_blurp("");

  enable_menu();

  mJobDirectory->SetRnrSelf(true);

  if(button) {
    button->MenuExit();
  }
}

/**************************************************************************/
/**************************************************************************/

void AEVDemoDriver::ConnectToProof()
{
  // This is vizualization only.
  // PROOF is connected during script processing as it is needed for
  // TDSet.Request().
  // For cosmic reasons the PROOF connection blocks all other processing.

  static const Exc_t _eh("AEVDemoDriver::ConnectToProof ");

  /*
  if(mProofTube != 0) throw(_eh + "PROOF already connected.");

  Float_t ttravel = 2.5;

  set_text("Connecting to PROOF ...");

  mProofTube = new WSTube("ProofTube");
  mProofTube->SetNodeA(*mDistan);
  mProofTube->SetNodeB(*mProof);
  mProofTube->SetLineW(3.5);
  mProofTube->SetColor(1, 0, 0.6);
  setup_tube(mProofTube, "Abstract1");
  setup_tube_for_travel(mProofTube, 5);

  mQueen->WriteLock();
  mQueen->CheckIn(mProofTube); Add(mProofTube);
  mQueen->WriteUnlock();

  mProofTube->AnimateConnect();

  set_text("Logging in ...");
  setup_tube_for_travel(mProofTube, ttravel);
  { auto_ptr<ZMIR> m( mProofTube->S_TravelAtoB() ); mSaturn->ShootMIR(m); }

  sleep(1.2 * ttravel);

  set_text("Authenticating ...");
  { auto_ptr<ZMIR> m(mProofTube->S_TravelBtoA()); mSaturn->ShootMIR(m); }
  sleep(0.5);
  { auto_ptr<ZMIR> m(mProofTube->S_TravelAtoB()); mSaturn->ShootMIR(m); }

  sleep(1.2 * ttravel);

  set_text("Connected to Proof.");
  { auto_ptr<ZMIR> m(mProofTube->S_StartAnimation()); mSaturn->ShootMIR(m); }
  */
}


void AEVDemoDriver::ConnectProofSlaves()
{
  static const Exc_t _eh("AEVDemoDriver::ConnectProofSlaves ");

  /*
  if(mProofTube == 0) throw(_eh + "PROOF not connected.");

  Float_t ttravel = 2.5;
  Float_t tconn   = 15;

  set_text("Sending job to PROOF ...");

  // { auto_ptr<ZMIR> m( mDistAnRep->S_ConnectSlaves() ); mSaturn->ShootMIR(m); }

  { auto_ptr<ZMIR> m( mProofTube->S_TravelAtoB() ); mSaturn->ShootMIR(m); }

  sleep(ttravel);

  list<AEVEventBatch*> ebs;
  mDistAnRep->CopyListByGlass<AEVEventBatch>(ebs);

  { auto_ptr<ZMIR> m( mDistAnRep->S_SendEvBatchesToTheatre() ); mSaturn->ShootMIR(m); }
  mTheatre->SetStepSleepMS(mSleepMS);
  { auto_ptr<ZMIR> m( mTheatre->S_StartHunt() ); mSaturn->ShootMIR(m); }

  rotate(*mProof, 5, 1, 2);

  set_text("Connecting slaves ...");

  TLorentzVector sgm(0.3, 0.3, 0.1, 0.1);
  for(list<AEVEventBatch*>::iterator eb=ebs.begin(); eb!=ebs.end(); ++eb) {
    string site( (*eb)->GetName() );
    AEVSiteViz* target = mMapViz->FindSiteViz(site.c_str());
    if(target == 0) continue;

    WSTube tube(GForm("Tube Proof-%s", site.c_str()));
    tube.SetNodeA(*mProof);
    tube.SetNodeB(target);
    // tube->SetColor(0, 1, 0);
    setup_tube(&tube, "Red2Yellow");
    setup_tube_for_travel(&tube, tconn);
    tube.SetSgmA(sgm);
    tube.SetSgmB(sgm);

    auto_ptr<ZMIR> mir (mQueen->S_IncarnateWAttach() );
    GledNS::StreamLens(*mir, &tube);
    auto_ptr<ZMIR> att_mir( S_Add(0) );
    mir->ChainMIR(att_mir.get());

    WSTube *tubeptr = 0;
    auto_ptr<ZMIR_RR> res( mSaturn->ShootMIRWaitResult(mir) );
    if(res->HasException()) {
      cout << _eh << "got exception: " << res->Exception.Data() << endl;
    }
    if(res->HasResult()) {
      ID_t id; *res >> id;
      tubeptr = dynamic_cast<WSTube*>(mSaturn->DemangleID(id));
    }

    if(tubeptr) {
      mProofMSTubes[site] = tubeptr;
      { auto_ptr<ZMIR> m( tubeptr->S_AnimateConnect() ); mSaturn->ShootMIR(m); }
    }
  }

  sleep(1.2*tconn);

  while(true) {
    GMutexHolder(mDistAnRep->mReadMutex);
    //mDistAnRep->ReadLock();
    bool done = true; // mDistAnRep->GetConnected();
    //mDistAnRep->ReadUnlock();
    if(done) break;
    sleep(0.5);
  }

  for(map<string, WSTube*>::iterator i=mProofMSTubes.begin();
      i!=mProofMSTubes.end(); ++i)
    {
      sleep(rnd(0.2));
      setup_tube_for_travel(i->second, ttravel);
      i->second->SetDefWidth(0.09);
      { auto_ptr<ZMIR> m( i->second->S_StartAnimation() ); mSaturn->ShootMIR(m); }
    }
  mProofTube->SetDefWidth(0.135);


  set_text("PROOF slaves connected.");
  */
}

void AEVDemoDriver::ProcessEvents()
{
  static const Exc_t _eh("AEVDemoDriver::ProcessEvents ");

  /*
  if(mProofTube == 0)       throw(_eh + "PROOF not connected.");
  if(mProofMSTubes.empty()) throw(_eh + "PROOF slaves not connected.");

  set_text("Event processing ...");

  mDistAnRep->WriteLock();
  mDistAnRep->StartProc();
  mDistAnRep->WriteUnlock();

  // sleep(2);

  mDistAnRep->Process();

  bool loop_on = true;
  // int i = 0;
  while(loop_on) {

    // printf("proof loop %d; ntodo=%d\n", ++i, mDistAnRep->RefBatchState().GetNToDo());

    map<string, int> chg;
    mDistAnRep->WriteLock();
    mDistAnRep->FakeProc(chg);
    mDistAnRep->WriteUnlock();

    mDistAnRep->WriteLock();
    if(mDistAnRep->RefBatchState().GetNToDo() == 0) loop_on = false;
    mDistAnRep->WriteUnlock();

    sleep(1);
  }
  */

  set_text("Event processing done.");
}

/**************************************************************************/

void AEVDemoDriver::VisProofProgress(map<string,int>& siteevmap)
{
  /*
  int sum = 0;
  for(map<string,int>::iterator i=siteevmap.begin(); i!=siteevmap.end(); ++i) {
    sum += i->second;

    map<string, WSTube*>::iterator t = mProofMSTubes.find(i->first);
    if(t == mProofMSTubes.end())
      continue;

    { auto_ptr<ZMIR> m( t->second->S_TravelBtoA() ); mSaturn->ShootMIR(m); }
  }

  if(sum > 0) {
    auto_ptr<ZMIR> m( mProofTube->S_TravelBtoA() );
    GTime t(GTime::I_Now);
    t += (Long_t) (1000*1.5*mSleepMS*mFPS);
    mSaturn->DelayedShootMIR(m, t);
  }
  */
}

/**************************************************************************/

void AEVDemoDriver::Report()
{}

/**************************************************************************/
