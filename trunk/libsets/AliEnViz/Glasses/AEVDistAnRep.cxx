// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AEVDistAnRep
//
//

#include "AEVDistAnRep.h"

#include "AEVEventBatch.h"
#include "AEVJobRep.h"
#include "AEVDemoDriver.h"

#include "AEVDistAnRep.c7"

#include "AEVSite.h"
#include "AEVSiteViz.h"
#include "AEVProcessViz.h"

#include <Glasses/ZQueen.h>
#include <Glasses/Eventor.h>
#include <Stones/ZComet.h>

#include <TPRegexp.h>
#include <TMath.h>

ClassImp(AEVDistAnRep);

/**************************************************************************/

void AEVDistAnRep::_init()
{
  mConnectTime = 2.5;
  mRotTime     = 2;
  mTravelTime  = 1.25;
  mWaitTime    = 0.05;

  mProcDuration = 0;

  mReplayDuration = 60;
  mReplayMaxWait  = 5;

  bInReplay       = false;
}

/**************************************************************************/

void AEVDistAnRep::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  if(mSites == 0) {
    ZNameMap* l = new ZNameMap("Sites", GForm("Containter of %s", GetName()));
    l->SetElementFID(AEVSite::FID());
    l->SetMIRActive(false);
    mQueen->CheckIn(l);
    SetSites(l);
  }
  if(mEvBatches == 0) {
    ZNameMap* l = new ZNameMap("EventBatches", GForm("Containter of %s", GetName()));
    l->SetElementFID(AEVEventBatch::FID());
    l->SetMIRActive(false);
    mQueen->CheckIn(l);
    SetEvBatches(l);
  }
  if(mProcVizes == 0) {
    ZNameMap* l = new ZNameMap("ProcVizes", GForm("Containter of %s", GetName()));
    l->SetElementFID(AEVProcessViz::FID());
    l->SetMIRActive(false);
    mQueen->CheckIn(l);
    SetProcVizes(l);
  }
}

void AEVDistAnRep::AddEventBatch(AEVEventBatch* evb)
{
  Add(evb);
  mEvBatches->Add(evb);
}

/**************************************************************************/

Int_t AEVDistAnRep::create_evbatches(map<string, int>& cmap)
{
  map<string, int>::iterator i = cmap.begin();
  int nres      = cmap.size();
  int totevents = 0;
  float dx = 1.5, x = -dx*nres/2, y = 2;
  while(i != cmap.end()) {
    Int_t events = i->second;
    printf("%-12s %d\n", i->first.c_str(), events);
    totevents += events;

    AEVEventBatch* eb = new AEVEventBatch(i->first.c_str(), GForm("EvBatch for %s", i->first.c_str()));
    eb->Reinit(events);
    eb->SetNWorkers(-1);
    eb->SetDataSizeMB(-1);

    mQueen->CheckIn(eb);
    AddEventBatch(eb);

    bool placed = false;
    if(mDemoDriver->GetMapViz() != 0) {
      placed = place_evbatch_on_map(eb);
    }
    if(!placed) {
      eb->SetPos(x, y, 0); x += dx;
    }

    ++i;
  }
  printf("Sites=%zd Events=%d\n", cmap.size(), totevents);
  return totevents;
}

void AEVDistAnRep::fix_evbatches()
{
  /*
  mDSet->GetListOfElementsMsn();
  TIter next_msn(mDSet->GetListOfElementsMsn());
  TDSetElementMsn* msn;
  while((msn = dynamic_cast<TDSetElementMsn*>(next_msn())) != 0) {
    Str_t msnname(msn->GetMsn());
    lStr_t els;
    GledNS::split_string(msnname, els, "::");
    if(els.size() != 3) {
      printf("Safr for %s\n", msnname.c_str());
      continue;
    }
    els.pop_front();
    AEVEventBatch* eb = dynamic_cast<AEVEventBatch*>
      (mEvBatches->GetElementByName( els.front().c_str() ));
    if(eb == 0) continue;

    SEvTaskState ts;
    ts.Reinit(msn->GetNfiles() * mMagicFac);
    eb->SetEvState(ts);

    eb->SetNWorkers(msn->GetNSiteDaemons());
    eb->SetDataSizeMB((Double_t)msn->GetDataSize()/1024/1024);
  }
  */
}

Bool_t AEVDistAnRep::place_evbatch_on_map(AEVEventBatch* eb)
{
  AEVSiteViz* sv = mDemoDriver->GetMapViz()->FindSiteViz(eb->GetName());
  if(sv == 0) return false;
  auto_ptr<ZTrans> t( ZNode::BtoA(this, sv) );
  if(t.get() == 0) return false;
  Double_t s[3];
  t->Unscale(s[0], s[1], s[2]);
  eb->SetTrans(*t);
  eb->SetScales(s[0], s[1], s[2]);
  return true;
}

/**************************************************************************/

void AEVDistAnRep::ResetJob()
{
  mBatchState.Reinit(mBatchState.GetNAll());
  mEventState.Reinit(mEventState.GetNAll());
  Stepper<AEVEventBatch> s(*mEvBatches);
  while(s.step())
    s->Reinit();
  mProcVizes->ClearList();
}

void AEVDistAnRep::ResetDistAn()
{
  ClearList();

  mSites->ClearList();
  mEvBatches->ClearList();
  mProcVizes->ClearList();

  mDemoDriver->GetMapViz()->ClearSiteVizes();
  mDemoDriver->GetTheatre()->ClearAmphitheatre();

  mBatchState.Reinit(0);
  mEventState.Reinit(0);

  Stamp(FID());
}

/**************************************************************************/

void AEVDistAnRep::InitJob(AEVJobRep* job)
{
  static const Exc_t _eh("AEVDistAnRep::InitJob ");

  if(job == 0 && mJobId == "")
    throw(_eh + "null argument.");
  if(job != 0) {
    mJobId   = job->GetName();
    mJobName = job->GetJobname();
  }

  Int_t total_entries = 0;

  {
    GLensWriteHolder wrlck(this);
    ResetDistAn();
  }

  AEVMlClient* mona_client = mDemoDriver->GetMonaClient();
  mona_client->SetCluster("ALIEN_QUERY");
  mona_client->SetNode(mJobId);
  mona_client->SetParam("sitelist");
  mona_client->SetFLSort(AEVMlClient::SM_Ascending);

  lMonaEntry_t l;

  mona_client->GetFLValues(l);
  if(l.empty())    throw(_eh + "empty sitelist.");
  if(l.size() > 1) throw(_eh + "got more than one result for sitelist.");

  // Setup query time.
  mQueryTime = l.front().fTime;

  TPMERegexp sites("\\s*:|,\\s*");
  Int_t n_sites = sites.Split(l.front().fValue);
  printf("n_sites=%d: %s\n", n_sites, l.front().fValue.Data());

  for(int s=0; s<n_sites; ++s) {
    mona_client->SetParam(sites[s] + ":param");

    l.clear();
    mona_client->GetFLValues(l);
    if(l.empty()) {
      printf("no data for %s\n", sites[s].Data());
      continue;
    }
    if(l.size() > 1) {
      printf("multiple entries for %s, using first\n", sites[s].Data());
    }

    TPMERegexp params("\\s*,\\s*");
    Int_t n_params = params.Split(l.front().fValue);
    printf("  site=%s, n_params=%d: %s\n", sites[s].Data(), n_params,
	   l.front().fValue.Data());

    AEVSite*       site    = new AEVSite(sites[s]);
    AEVEventBatch* evbatch = new AEVEventBatch(sites[s], GForm("EvBatch for %s", sites[s].Data()));

    TPMERegexp keyval("\\s*:\\s*");
    for(int p=0; p<n_params; ++p) {
      Int_t n = keyval.Split(params[p]);
      if(n != 2) {
	printf("split problems with %s\n", params[p].Data());
	continue;
      }
      printf("    '%s' = '%s'\n", keyval[0].Data(), keyval[1].Data());

      if(keyval[0] == "location") {
	site->SetLocation(keyval[1]);
      }
      // !!! Watch this !!! Wrong stuff stored in monalisa.
      else if(keyval[0] == "longitude") {
	site->SetLatitude(atof(keyval[1]));
      }
      else if(keyval[0] == "latitude") {
	site->SetLongitude(atof(keyval[1]));
      }
      else if(keyval[0] == "entries") {
	Int_t ent = atoi(keyval[1]);
	total_entries += ent;
	evbatch->Reinit(ent);
      }
      else if(keyval[0] == "datasize") {
	Double_t siz = atol(keyval[1])/1024.0/1024.0;
	evbatch->SetDataSizeMB(siz);
      }
      else {
	printf("unknown paramater '%s'\n", keyval[0].Data());
      }
    }

    GLensWriteHolder wrlck(this);

    mQueen->CheckIn(site);    mSites->Add(site);
    mQueen->CheckIn(evbatch); AddEventBatch(evbatch);

    if(mDemoDriver->GetMapViz()->ImportSite(site) == false)
      ISerr(_eh + "site '" + sites[s] + "' not placed on map.");
    place_evbatch_on_map(evbatch);
  }

  GLensWriteHolder wrlck(this);

  mBatchState.Reinit(n_sites);
  mEventState.Reinit(total_entries);

  Stamp(FID());
}

namespace
{

struct SiteData
{
  const char* name;
  const char* location;
  float       longitude;
  float       latitude;
};

SiteData SiteDatas[] = {
  { "Muenster", "Muenster", 7.633, 51.967 },
  { "ITEP", "Moscow", 37.42, 55.45 },
  // { "Clermont", "Clermont-Ferrand", 3.05, 44.46 },
  { "RMKI", "Budapest", 19.5, 47.30 },
  { "CERN", "Geneva", 6.15, 46.217 },
  { "CNAF", "Bologna", 11.333, 44.5 },
  { "Bari", "Bari", 16.867, 41.117 },
  { "Catania", "Catania", 15.1, 37.517 },
  { "Prague", "Prague", 14.26, 50.10 },
  { "FZK", "Karlsruhe", 8.4, 49.05 },
  { "Houston", "Houston", -95.33, 29.7500 },
  { "PNPI", "St. Petersburg", 30.2500, 58.8833 },
};
Int_t SiteDatasN = 11;

}

void AEVDistAnRep::FakeInitJob(AEVJobRep* job)
{
  static const Exc_t _eh("AEVDistAnRep::FakeInitJob ");

  if(job == 0 && mJobId == "")
    throw(_eh + "null argument.");
  if(job != 0) {
    mJobId   = job->GetName();
    mJobName = job->GetJobname();
  }

  Int_t total_entries = 0;

  {
    GLensWriteHolder wrlck(this);
    ResetDistAn();
  }

  TRandom gen(0);

  lMonaEntry_t l;

  Int_t n_sites = SiteDatasN;

  for(int s=0; s<n_sites; ++s) {
    AEVSite*       site    = new AEVSite(SiteDatas[s].name);
    AEVEventBatch* evbatch = new AEVEventBatch
      (SiteDatas[s].name, GForm("EvBatch for %s", SiteDatas[s].name));

    site->SetLocation(SiteDatas[s].location);
    site->SetLatitude(SiteDatas[s].latitude);
    site->SetLongitude(SiteDatas[s].longitude);
    {
      Int_t ent = Int_t(13 + 47*gen.Rndm());
      total_entries += ent;
      evbatch->Reinit(ent);
      Double_t siz = ent*100*(1 + 0.4*(gen.Rndm()-0.5));
      evbatch->SetDataSizeMB(siz);
      evbatch->SetProcAvg(TMath::Nint(2 + 2*gen.Rndm()));
    }

    GLensWriteHolder wrlck(this);

    mQueen->CheckIn(site);    mSites->Add(site);
    mQueen->CheckIn(evbatch); AddEventBatch(evbatch);

    if(mDemoDriver->GetMapViz()->ImportSite(site) == false)
      ISerr(_eh + "site '" + SiteDatas[s].name + "' not placed on map.");
    place_evbatch_on_map(evbatch);
  }

  GLensWriteHolder wrlck(this);

  mBatchState.Reinit(n_sites);
  mEventState.Reinit(total_entries);

  Stamp(FID());
}

/**************************************************************************/

void AEVDistAnRep::StudyJobHistory(Bool_t dump_p)
{
  static const Exc_t _eh("AEVDistAnRep::StudyJobHistory ");

  if(mJobId == "")
    throw(_eh + "no job specified.");

  AEVMlClient* mona_client = mDemoDriver->GetMonaClient();
  mona_client->SetCluster("ALIEN_PROCESS");
  mona_client->SetNode(mJobId);
  mona_client->SetParam("*");

  mona_client->SetFromHrs(mQueryTime.TimeUntilNow().ToDouble()/3600);
  mona_client->SetToHrs(0);

  mHistory.clear();
  mona_client->GetValues(mHistory);

  if(mHistory.empty() == false) {
    mProcStart    = mHistory.front().fTime;
    mProcEnd      = mHistory.back().fTime;
    mProcDuration = (mProcEnd - mProcStart).ToDouble();
  } else {
    mProcStart    = 0l;
    mProcEnd      = 0l;
    mProcDuration = 0;
  }

  Stamp(FID());

  if(dump_p) {
    printf("%sjobname='%s' jobid='%s' entries=%zd\n", _eh.Data(),
	   mJobName.Data(), mJobId.Data(), mHistory.size());
    for(lMonaEntry_i i=mHistory.begin(); i!=mHistory.end(); ++i)
      printf("  %-32s %12s %s\n", i->fParam.Data(), i->fValue.Data(), i->fDateStr.Data());
  }
}

/**************************************************************************/

AEVProcessViz* AEVDistAnRep::find_or_crete_procviz(const TString& proc,
						   const TString& site)
{
  static const Exc_t _eh("AEVDistAnRep::find_or_crete_procviz ");

  AEVProcessViz* pv = (AEVProcessViz*) mProcVizes->GetElementByName(proc);
  if(pv == 0) {
    AEVSiteViz* sv = mDemoDriver->GetMapViz()->FindSiteViz(site);
    if(sv == 0)
      throw(_eh + "site-viz for '" + site + "' not found.");

    AEVEventBatch* eb = (AEVEventBatch*) mEvBatches->GetElementByName(site);
    if(eb == 0)
      throw(_eh + "event-batch for '" + site + "' not found.");

    pv = new AEVProcessViz(proc);
    mQueen->CheckIn(pv);
    pv->SetParent(this);
    pv->SetNodeA(sv);
    pv->SetNodeB(mDemoDriver->GetMonaViz()->GetParent());
    pv->SetBatch(eb);
    // mDemoDriver->setup_tube(pv, "PROOF");
    mDemoDriver->setup_tube(pv, "Red2Yellow");
    pv->SetTexUScale(5);
    pv->SetDtexU(-0.5);
    pv->SetDefVelocity(1 / mTravelTime);
    pv->SetMinWaitTime(mWaitTime);
    mDemoDriver->animate_tube(pv);
    {
      GLensWriteHolder wrlck(this);
      mProcVizes->Add(pv);
    }
    pv->AnimateConnect(1 / mConnectTime);
    {
      GLensWriteHolder wrlck(eb);
      eb->IncProcessing();
    }
  }

  return pv;
}

Int_t AEVDistAnRep::ReplayJobFromHistory()
{
  // Returns number of remaining entries (also in mir-result-request).

  static const Exc_t _eh("AEVDistAnRep::ReplayJobFromHistory ");

  GMutexHolder replay_lock(mReplayCond);
  {
    GLensWriteHolder wrlck(this);
    if(bInReplay)
      throw(_eh + "already playing.");
    if(mHistory.empty())
      throw(_eh + "job history empty.");

    SetInReplay(true);
    mReplayTime = mHistory.front().fTime;
  }

  TPMERegexp proc_parname_re("([^:]+:[^:]+:[^:]+):([^:]+)");
  TPMERegexp site_subproc_re("([^:]+):([^:]+:[^:]+)");
  while(true) {
    if(mHistory.empty()) {
      GLensWriteHolder wrlck(this);
      printf("history empty, returning\n");
      SetInReplay(false);
      break;
    }

    AEVMlClient::MonaEntry me = mHistory.front();

    if(me.fTime > mReplayTime) {
      Double_t sleep_sec = (me.fTime - mReplayTime).ToDouble() *
	mReplayDuration / mProcDuration;
      if(sleep_sec > mReplayMaxWait)
	sleep_sec = mReplayMaxWait;
      printf("sleeping for %.3lf ms\n", sleep_sec);
      mReplayCond.TimedWaitMS(UInt_t(1000*sleep_sec));

      GLensWriteHolder wrlck(this);
      if(bInReplay == false) {
	printf("replay stopped, returning\n");
	break;
      }
      mReplayTime = me.fTime;
      mReplayPos  = (mReplayTime - mProcStart).ToDouble()*
	mReplayDuration/mProcDuration;
      Stamp(FID());
    }
    mHistory.pop_front();

    printf("processing: '%s'\n", me.StdFormat());
    mDemoDriver->set_blurp(me.fDateStr);
    try {

      if(me.fParam == "status") {
	if(me.fValue == "STARTED") {
	  continue;
	}
	else if(me.fValue == "DONE") {
	  // This is not present
	  // !!! finalize all
	  // ??? break ??? history should be empty anyway
	}
	// RESET ???
	// STARTED can be ignored
	continue;
      }

      Int_t m1, m2;
      m1 = proc_parname_re.Match(me.fParam);
      if(m1 != 3) {
	printf("  m1=%d trouble, skipping\n", m1);
	continue;
      }
      printf("  proc_id='%s', subparam='%s'\n",
	     proc_parname_re[1].Data(), proc_parname_re[2].Data());
      m2 = site_subproc_re.Match(proc_parname_re[1]);
      if(m2 != 3) {
	printf("  m2=%d trouble, skipping\n", m2);
	continue;
      }
      printf("  site='%s', subproc='%s'\n",
	     site_subproc_re[1].Data(), site_subproc_re[2].Data());

      TString proc    = proc_parname_re[1];
      TString parname = proc_parname_re[2];
      TString site    = site_subproc_re[1];
      TString subproc = site_subproc_re[2];

      AEVProcessViz* pv = find_or_crete_procviz(proc, site);
      AEVEventBatch* eb = pv->GetBatch();

      if(parname == "status") {
	printf("  status = %s\n", me.fValue.Data());
	if(me.fValue == "PROCESSING") {
	  // All is done when first record for process is received.
	  // See above.
	}
	else if(me.fValue == "DONE") {
	  { GLensWriteHolder wrlck(eb);
	    eb->DecProcessing();

	    if(eb->IsDone()) {
	      eb->Finalize();
	      mBatchState.IncNOK(1);
	      Stamp(FID());
	    }
	  }
	  { GLensWriteHolder wrlck(pv);
	    pv->AnimateDisconnect(1 / mConnectTime, true);
	  }
	}
      }
      else if(parname == "entries") {
	Int_t ents  = atoi(me.fValue.Data());
	Int_t delta = ents - pv->GetEntsDone();
	printf("  entries = %d, delta = %d\n", ents, delta);
	if(delta != 0) {
	  eb->mEvState.IncNOK(delta);
	  eb->Stamp(AEVEventBatch::FID());
	  mEventState.IncNOK(delta);
	  Stamp(FID());
	  pv->SetEntsDone(ents);
	  {
	    GLensWriteHolder tube_lck(pv);
	    pv->MakeTraveler();
	  }
	}
      }
      else if(parname == "datasize") {
	Float_t megs  = atof(me.fValue.Data())/1024/1024;
	Float_t delta = megs - pv->GetMegsDone();
	printf("  megs = %lf\n", megs);
	if(delta != 0) {
	  eb->mDataDoneMB = megs;
	  eb->Stamp(AEVEventBatch::FID());
	  pv->SetMegsDone(megs);
	}
      }
      else {
	printf("  unknown parameter name\n");
      }

    } catch(Exc_t exc) {
      printf("exception: '%s'.\nproceeding ...\n", exc.Data());
    }
  }

  GLensWriteHolder wrlck(this);
  Int_t n_left = mHistory.size();

  SetInReplay(false);

  ZMIR* mir = get_MIR();
  if(mir && mir->HasResultReq()) {
    TBufferFile b(TBuffer::kWrite);
    b << n_left;
    mSaturn->ShootMIRResult(b);
  }

  return n_left;
}

namespace {

struct EvBatchInfo {
  AEVEventBatch*         event_batch;
  vector<AEVProcessViz*> proc_vector;
  Int_t                  all_proc_count;
  Int_t                  done_proc_count;

  EvBatchInfo(AEVEventBatch* eb) : event_batch(eb)
  { all_proc_count = done_proc_count = 0;}
};

};

Int_t AEVDistAnRep::FakeReplayJobFromHistory()
{
  // Returns number of remaining entries (also in mir-result-request).

  static const Exc_t _eh("AEVDistAnRep::FakeReplayJobFromHistory ");

  GMutexHolder replay_lock(mReplayCond);
  {
    GLensWriteHolder wrlck(this);
    if(bInReplay)
      throw(_eh + "already playing.");

    SetInReplay(true);
    // mReplayTime.SetNow();
  }

  TRandom gen(0);

  Int_t n_todo = 0;
  list<EvBatchInfo> eb_infos;

  Stepper<AEVEventBatch> ebs(*mEvBatches);
  while(ebs.step()) {
    n_todo += ebs->RefEvState().GetNToDo();
    eb_infos.push_back(EvBatchInfo(*ebs));
  }

  while(true) {
    // printf("= n_todo=%d ============================================\n", n_todo);

    if(n_todo == 0) {
      GLensWriteHolder wrlck(this);
      printf("fake seems completed, returning\n");
      mEventState.SetNProc(0);
      SetInReplay(false);
      break;
    }
    n_todo = 0; // Actual value accumulated on each pass.

    for(list<EvBatchInfo>::iterator ebi=eb_infos.begin(); ebi!=eb_infos.end(); ++ebi) {
      AEVEventBatch* eb = ebi->event_batch;
      Int_t ex_nproc = eb->RefEvState().GetNProc();
      Int_t ex_ndone = eb->RefEvState().GetNDone();
      Int_t ex_nok   = eb->RefEvState().GetNOK();
      Int_t ex_nfail = eb->RefEvState().GetNFail();

      eb->FakeProc();

      Int_t nproc = eb->RefEvState().GetNProc();
      Int_t ndone = eb->RefEvState().GetNDone();
      Int_t nok   = eb->RefEvState().GetNOK();
      Int_t nfail = eb->RefEvState().GetNFail();

      Int_t delta_nproc = nproc - ex_nproc;
      Int_t delta_ndone = ndone - ex_ndone;
      Int_t delta_nok   = nok   - ex_nok;
      Int_t delta_nfail = nfail - ex_nfail;

      // printf("Site=%-12s ExNproc=%3d Nproc=%3d Dproc=%3d, ExNdone=%3d Ndone=%3d Ddone=%3d\n",
      //   eb->GetName(), ex_nproc, nproc, delta_nproc,
      //   ex_ndone, ndone, delta_ndone);

      Int_t born_procs = delta_nproc;
      while(born_procs-- > 0) {
	TString new_name(GForm("%s-%d", eb->GetName(), ebi->all_proc_count++));
	AEVProcessViz* pv = 0;
	{
	  AEVSiteViz* sv = mDemoDriver->GetMapViz()->FindSiteViz(eb->GetName());
	  if(sv == 0)
	    throw(_eh + "site-viz for '" + eb->GetName() + "' not found.");

	  pv = new AEVProcessViz(new_name);
	  mQueen->CheckIn(pv);
	  pv->SetParent(this);
	  pv->SetNodeA(sv);
	  pv->SetNodeB(mDemoDriver->GetMonaViz()->GetParent());
	  pv->SetBatch(eb);
	  // mDemoDriver->setup_tube(pv, "PROOF");
	  mDemoDriver->setup_tube(pv, "Red2Yellow");
	  pv->SetTexUScale(5);
	  pv->SetDtexU(-0.5);
	  pv->SetDefVelocity(1 / mTravelTime);
	  pv->SetMinWaitTime(mWaitTime);
	  mDemoDriver->animate_tube(pv);
	  {
	    GLensWriteHolder wrlck(this);
	    mProcVizes->Add(pv);
	  }
	  GLensWriteHolder wrlck(pv);
	  pv->AnimateConnect(1 / mConnectTime);
	}
	ebi->proc_vector.push_back(pv);
      }
      Int_t balls = delta_ndone;
      while(balls-- > 0) {
	int nproc = ebi->all_proc_count  - ebi->done_proc_count;
	int pos   = ebi->done_proc_count + int(nproc*gen.Rndm());
	// printf("  Shoot: pos=%d ndone=%d nall=%d\n", pos, ebi->done_proc_count, ebi->all_proc_count);
	AEVProcessViz* pv = ebi->proc_vector[pos];
	GLensWriteHolder tube_lck(pv);
	pv->MakeTraveler();
      }
      Int_t died_procs = delta_nproc;
      while(died_procs++ < 0) {
	int pos = ebi->done_proc_count++;
	AEVProcessViz* pv = ebi->proc_vector[pos];
	GLensWriteHolder wrlck(pv);
	pv->AnimateDisconnect(1 / mConnectTime, true);
      }

      if(delta_ndone != 0) {
	mEventState.IncNOK(delta_nok);
	mEventState.IncNFail(delta_nfail);
	mEventState.IncNProc(delta_nproc);
	Stamp(FID());
      }

      if(eb->RefEvState().GetNToDo() == 0) {
	// printf("Site='%s' FINISHED\n", eb->GetName());
	list<EvBatchInfo>::iterator i = ebi--;
	// assert no more connections?
	eb_infos.erase(i);
	{ GLensWriteHolder wrlck(eb);
	  eb->Finalize();
	  mBatchState.IncNOK(1);
	  Stamp(FID());
	}
      }

      n_todo += eb->RefEvState().GetNToDo();
    }

    // printf("=======================================================\n");
    {
      Eventor* etor = mDemoDriver->GetAnimator();
      Double_t tstart = etor->GetInternalTime();
      do {
	mReplayCond.TimedWaitMS(UInt_t(200));
	GLensWriteHolder wrlck(this);
	if(bInReplay == false) {
	  printf("replay stopped, returning\n");
	  break;
	}
      } while (etor->GetInternalTime() - tstart < 1.4);
    }

  }

  GLensWriteHolder wrlck(this);

  SetInReplay(false);

  ZMIR* mir = get_MIR();
  if(mir && mir->HasResultReq()) {
    TBufferFile b(TBuffer::kWrite);
    b << n_todo;
    mSaturn->ShootMIRResult(b);
  }

  return n_todo;
}

/**************************************************************************/

void AEVDistAnRep::StopReplay()
{
  static const Exc_t _eh("AEVDistAnRep::StopReplay ");

  if(bInReplay == false)
    throw(_eh + "not playing.");

  SetInReplay(false);
  mReplayCond.LockSignal();
}

void AEVDistAnRep::FinalizeJob()
{
  static const Exc_t _eh("AEVDistAnRep::FinalizeJob ");

  if(bInReplay)
    throw(_eh + "replay not finished.");

  { GMutexHolder llck(mProcVizes->RefListMutex());
    Stepper<AEVProcessViz> s(*mProcVizes);
    while(s.step()) {
      if(s->GetConnectionStauts() == WSTube::CS_Connected) {
	AEVEventBatch* eb = s->GetBatch();
	{ GLensWriteHolder wrlck(eb);
	  eb->DecProcessing();
	}
	{ GLensWriteHolder wrlck(*s);
	  s->AnimateDisconnect(1 / mConnectTime, true);
	}
      }
    }
  }

  { GMutexHolder llck(mEvBatches->RefListMutex());
    Stepper<AEVEventBatch> s(*mEvBatches);
    while(s.step()) {
      if(s->RefEvState().GetState() != 'F') {
	GLensWriteHolder wrlck(*s);
	s->Finalize();
	if(s->RefEvState().GetNOK() > s->RefEvState().GetNAll()/2) {
	  mBatchState.IncNOK(1);
	} else {
	  mBatchState.IncNFail(1);
	}
      }
    }
  }

  mBatchState.Finalize();
  mEventState.Finalize();
  Stamp(FID());
}

/**************************************************************************/

namespace {

struct ProcInfo {
  TString fSite;
  TString fStatus;
  Int_t   fEntsDone;
  Float_t fMegsDone;

  ProcInfo() : fEntsDone(0), fMegsDone(0) {}
};

}

void AEVDistAnRep::FollowJob()
{
  static const Exc_t _eh("AEVDistAnRep::FollowJob ");

  AEVMlClient* mona_client = mDemoDriver->GetMonaClient();
  mona_client->SetCluster("ALIEN_PROCESS");
  mona_client->SetNode(mJobId);
  mona_client->SetParam("*");

  TPMERegexp proc_parname_re("([^:]+:[^:]+:[^:]+):([^:]+)");
  TPMERegexp site_subproc_re("([^:]+):([^:]+:[^:]+)");

  map<TString, ProcInfo> proc_map;
  Bool_t first_entry = true;

  while(bInReplay) {

    mHistory.clear();
    proc_map.clear();
    mona_client->GetFLValues(mHistory);

    for(lMonaEntry_i mei=mHistory.begin(); mei!=mHistory.end(); ++mei) {
      AEVMlClient::MonaEntry& me = *mei;

      if(me.fParam == "status") {
	if(me.fValue == "STARTED") {
	  continue;
	}
	else if(me.fValue == "DONE") {
	  // This is not present
	  // !!! finalize all
	  // ??? break ??? history should be empty anyway
	}
	// RESET ???
	// STARTED can be ignored
	continue;
      }

      Int_t m1, m2;
      m1 = proc_parname_re.Match(me.fParam);
      if(m1 != 3) {
	printf("  m1=%d trouble, skipping\n", m1);
	continue;
      }
      printf("  proc_id='%s', subparam='%s'\n",
	     proc_parname_re[1].Data(), proc_parname_re[2].Data());
      m2 = site_subproc_re.Match(proc_parname_re[1]);
      if(m2 != 3) {
	printf("  m2=%d trouble, skipping\n", m2);
	continue;
      }
      printf("  site='%s', subproc='%s'\n",
	     site_subproc_re[1].Data(), site_subproc_re[2].Data());

      TString proc    = proc_parname_re[1];
      TString parname = proc_parname_re[2];
      TString site    = site_subproc_re[1];
      TString subproc = site_subproc_re[2];

      proc_map[proc].fSite = site;
      if(parname == "status")
	proc_map[proc].fStatus = me.fValue;
      else if(parname == "entries")
	proc_map[proc].fEntsDone = atoi(me.fValue.Data());
      else if(parname == "datasize")
	proc_map[proc].fMegsDone = atof(me.fValue.Data())/1024/1024;

    } // for mona entry

    for(map<TString, ProcInfo>::iterator pimi = proc_map.begin();
	pimi != proc_map.end(); ++pimi)
      {
	const TString& proc = pimi->first;
	ProcInfo&      pi   = pimi->second;

	AEVProcessViz* pv = 0;
	if(pi.fStatus == "PROCESSING")
	  pv = find_or_crete_procviz(proc, pi.fSite);
	else
	  pv = (AEVProcessViz*) mProcVizes->GetElementByName(proc);
	AEVEventBatch* eb = (AEVEventBatch*) mEvBatches->GetElementByName(pi.fSite);

	if(pv != 0) {
	  { // check num entries
	    Int_t ents  = pi.fEntsDone;
	    Int_t delta = ents - pv->GetEntsDone();
	    if(delta != 0) {
	      printf("  entries = %d, delta = %d\n", ents, delta);
	      eb->mEvState.IncNOK(delta);
	      eb->Stamp(AEVEventBatch::FID());
	      mEventState.IncNOK(delta);
	      Stamp(FID());
	      pv->SetEntsDone(ents);
	      {
		GLensWriteHolder tube_lck(pv);
		pv->MakeTraveler();
	      }
	    }
	  }
	  { // check num megs
	    Float_t megs  = pi.fMegsDone;
	    Float_t delta = megs - pv->GetMegsDone();
	    printf("  megs = %lf\n", megs);
	    if(delta != 0) {
	      eb->mDataDoneMB = megs;
	      eb->Stamp(AEVEventBatch::FID());
	      pv->SetMegsDone(megs);
	    }
	  }
	}

	if(pi.fStatus == "DONE") {
	  if(first_entry) {
	    Int_t delta = pi.fEntsDone;
	    eb->mEvState.IncNOK(delta);
	    eb->Stamp(AEVEventBatch::FID());
	    mEventState.IncNOK(delta);
	    Stamp(FID());
	  } else {
	    // if in procs, disconnect
	    if(pv != 0) {
	      { GLensWriteHolder wrlck(pv);
		pv->AnimateDisconnect(1 / mConnectTime, true);
	      }
	      { GLensWriteHolder wrlck(eb);
		eb->DecProcessing();
	      }
	    }
	  }
	} // if status == done

	{ GLensWriteHolder wrlck(eb);
	  if(eb->IsDone()) {
	    eb->Finalize();
	    mBatchState.IncNOK(1);
	    Stamp(FID());
	  }
	}

      } // for process map

    first_entry = false;

  } //while in replay
}

/**************************************************************************/
/**************************************************************************/

void AEVDistAnRep::SendEvBatchesToTheatre(Amphitheatre* amph)
{
  static const Exc_t _eh("AEVDistAnRep::SendEvBatchesToTheatre ");

  if(amph == 0) amph = mDemoDriver->GetTheatre();
  if(amph == 0) throw(_eh + "no theatre in sight.");

  list<AEVEventBatch*> ebs;
  CopyListByGlass<AEVEventBatch>(ebs);
  amph->WriteLock();
  for(list<AEVEventBatch*>::iterator eb=ebs.begin(); eb!=ebs.end(); ++eb) {
    amph->AddGuest(*eb);
    RemoveAll(*eb);
  }
  amph->WriteUnlock();
  Stamp(FID());

  mSaturn->ShootMIR( amph->S_StartHunt() );
}

/**************************************************************************/
/**************************************************************************/

void AEVDistAnRep::Process()
{
  static const Exc_t _eh("AEVDistAnRep::Process ");

  /*
  if(mDSet == 0)          throw(_eh + "DataSet not present.");
  if(bConnected == false) throw(_eh + "DatSet not Connected to PROOF.");

  {
    TList* l = new TList();
    l->SetName("FeedbackList");
    l->Add(new TObjString("hMassK0"));
    mDSet->AddInput(l);
  }

  WriteLock();
  SetProcessing(true);
  _first_feedback = true;
  mFeedback.Connect(mProof, this);
  WriteUnlock();

  Int_t nproc = 0;
  list<AEVEventBatch*> ebs;
  mEvBatches->CopyListByGlass<AEVEventBatch*>(ebs);
  for(list<AEVEventBatch*>::iterator eb=ebs.begin(); eb!=ebs.end(); ++eb) {
    (*eb)->WriteLock();
    SEvTaskState es = (*eb)->RefEvState();
    es.SetNProc((*eb)->GetNWorkers() <? es.GetNAll());
    nproc += es.GetNProc();
    es.SetState('R');
    (*eb)->SetEvState(es);
    (*eb)->WriteUnlock();
  }

  WriteLock();
  mBatchState.SetNProc(mBatchState.GetNAll());
  mBatchState.SetState('R');
  mEventState.SetNProc(nproc);
  mEventState.SetState('R');

  Stamp(FID());
  WriteUnlock();

  mProof->EnablePackage(mPkgName.Data());
  mProof->Exec(GForm(".include ../packages/%s", mPkgName.Data()));

  mDSet->Process(mSelector.Data());

  FinalizeProcStatus();

  WriteLock();
  mFeedback.Disconnect();
  SetProcessing(false);
  WriteUnlock();
  */
}

void AEVDistAnRep::UpdateProcStatus(TList* siteinfos,
				    map<string,int>* siteevmap)
{
  /*
  Int_t d_evs = 0, d_proc = 0, d_sites = 0;

  if(_first_feedback) {
    fix_evbatches();
    _first_feedback = false;
  }

  TIter next_site(siteinfos);
  TObject* ro;

  int site_count = 0;
  while((ro = next_site()) != 0) {
    ++site_count;
    TSiteNodeInfo* sni = dynamic_cast<TSiteNodeInfo*>(ro);
    if (sni == 0) continue;
    Str_t cename(sni->GetNodeName());
    Str_t sitename;
    {
      lStr_t els;
      GledNS::split_string(cename, els, "::");
      if(els.size() != 3) {
	printf("Safr for %s\n", cename.c_str());
	continue;
      }
      els.pop_front();
      sitename = els.front();
    }
    AEVEventBatch* eb = dynamic_cast<AEVEventBatch*>
      (mEvBatches->GetElementByName( sitename.c_str() ));
    if(eb == 0) continue;

    // sni->Print();

    eb->WriteLock();
    SEvTaskState ts = eb->GetEvState();

    if(ts.GetNToDo() != 0) {

      Int_t ndone =  (Int_t)sni->GetEventsProcessed();

      if(ts.GetNOK() < ndone) {
	Int_t delta = ndone - ts.GetNOK();
	d_evs += delta;
	ts.SetNOK(ndone);
	if(ts.GetNToDo() < ts.GetNProc()) {
	  d_proc += ts.GetNProc() - ts.GetNToDo();
	  ts.SetNProc(ts.GetNToDo());
	}
	if(ts.GetNToDo() <= 0 && ts.GetState() != 'F') {
	  d_sites += 1;
	  ts.SetState('F');
	}
	eb->SetEvState(ts);
	if(siteevmap) (*siteevmap)[sitename] = delta;
      }
    }
    eb->WriteUnlock();
  }

  // printf("Final: %d, %d, %d\n", d_evs, d_proc, d_sites);

  GLensWriteHolder wrlck(this);

  mBatchState.IncNOK(d_sites);
  mBatchState.IncNProc(-d_sites);

  mEventState.IncNOK(d_evs);
  mEventState.IncNProc(-d_proc);

  Stamp(FID());
  */
}

/**************************************************************************/

void AEVDistAnRep::StartProc()
{
  mBatchState.SetNProc(mBatchState.GetNAll());

  list<AEVEventBatch*> ebs;
  mEvBatches->CopyListByGlass<AEVEventBatch>(ebs);
  for(list<AEVEventBatch*>::iterator eb=ebs.begin(); eb!=ebs.end(); ++eb) {
    (*eb)->WriteLock();
    const SEvTaskState&	es((*eb)->RefEvState());
    (*eb)->SetProcAvg((Int_t) TMath::Floor(es.GetNAll()*0.1 + 1));
    (*eb)->SetFracFail(0.15);
    (*eb)->WriteUnlock();
  }
  Stamp(FID());
}

void AEVDistAnRep::FakeProc()
{
  map<string, int> changes;
  FakeProc(changes);
}

void AEVDistAnRep::FakeProc(map<string, int>& changes)
{
  list<AEVEventBatch*> ebs;
  mEvBatches->CopyListByGlass<AEVEventBatch>(ebs);
  for(list<AEVEventBatch*>::iterator eb=ebs.begin(); eb!=ebs.end(); ++eb) {
    (*eb)->WriteLock();
    SEvTaskState s0( (*eb)->RefEvState() );
    if(s0.GetNToDo() > 0) {
      (*eb)->FakeProc();
      SEvTaskState s1( (*eb)->RefEvState() );
      int delta = s1.GetNDone() - s0.GetNDone();
      changes[(*eb)->GetName()] = delta;

      if(s1.GetNToDo() == 0) {
	mBatchState.IncNOK(1);
	mBatchState.IncNProc(-1);
      }
      { int d = s1.GetNOK()   - s0.GetNOK();   mEventState.IncNOK(d); }
      { int d = s1.GetNFail() - s0.GetNFail(); mEventState.IncNFail(d); }
      { int d = s1.GetNProc() - s0.GetNProc(); mEventState.IncNProc(d); }
    }
    (*eb)->WriteUnlock();
  }
  Stamp(FID());
}
