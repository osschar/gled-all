// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZAliLoad
//
//

#include "ZAliLoad.h"
#include "ZAliLoad.c7"
#include <Glasses/ZQueen.h>
#include <Glasses/RecTrack.h>
#include <Glasses/V0.h>
#include <Stones/TTreeTools.h>

#include <TSystem.h>
#include <TBranchElement.h>
#include <TEventList.h>
#include <TParticle.h>

#include <AliRun.h>
#include <AliHit.h>
#include <AliTPCTrackHitsV2.h>
#include <AliPDG.h>
#include <AliTPCParam.h>
#include <AliSimDigits.h>
#include <AliKalmanTrack.h>
#include <AliESD.h>
#include <AliTPCclusterMI.h>
#include <AliTPCClustersRow.h>

typedef list<ZParticle*>           lpZATrack_t;
typedef list<ZParticle*>::iterator lpZATrack_i;

ClassImp(ZAliLoad)

/**************************************************************************/
// Operation Mutex holder
// Prevents simultaneous execution of detached threads.
// Should provide optional queuing.
/**************************************************************************/

namespace {
  
  class OpMutexHolder {
    ZAliLoad* mAli;
    TString   mOldOp;
  public:
    OpMutexHolder(ZAliLoad* a, const Text_t* op) : mAli(a)
    {
      if(mAli->RefOpMutex().TryLock() != GMutex::ok) {
	throw(string(GForm("%s rejected (command queuing not implemented).", op)));
      }
      mOldOp = mAli->GetOperation();
      mAli->SetOperation(op);
    }

    ~OpMutexHolder()
    {
      mAli->SetOperation(mOldOp.Data());
      mAli->RefOpMutex().Unlock();
    }
  };

}

/**************************************************************************/

void ZAliLoad::_init()
{
  // *** Set all links to 0 ***
  mTreeK  = 0;
  mTreeH  = 0;
  mTreeTR = 0;
  mTreeC  = 0;
  mTreeR  = 0;
  mpP = &mP;
  mpH = &mH;
  mpC = &mC;
  mpR = &mR;
  mTPCDigInfo = 0;

  mDataDir = ".";
  mEvent   = 0;

  mVSDName = "AliVSD.root";
  mFile    = 0; mDirectory = 0;

  mOperation = "<idle>";

  mParticleSelection = "GetMother(0) == -1";
  mHitSelection      = "GetDetID() < 5";
  mClusterSelection  = mHitSelection;
  mRecTrackSelection = "GetLabel() >= 0";
  pRunLoader = 0;

  // Pain:
  AliPDG::AddParticlesToPdgDataBase();
  {
    TDatabasePDG *pdgDB = TDatabasePDG::Instance();
    // const Int_t kspe=50000000;
    const Int_t kion=10000000;

    const Double_t kAu2Gev=0.9314943228;
    const Double_t khSlash = 1.0545726663e-27;
    const Double_t kErg2Gev = 1/1.6021773349e-3;
    const Double_t khShGev = khSlash*kErg2Gev;
    const Double_t kYear2Sec = 3600*24*365.25;

    pdgDB->AddParticle("Deuteron","Deuteron",2*kAu2Gev+8.071e-3,kTRUE,
		       0,1,"Ion",kion+10020);
    pdgDB->AddParticle("Triton","Triton",3*kAu2Gev+14.931e-3,kFALSE,
		       khShGev/(12.33*kYear2Sec),1,"Ion",kion+10030);
    pdgDB->AddParticle("Alpha","Alpha",4*kAu2Gev+2.424e-3,kTRUE,
		       khShGev/(12.33*kYear2Sec),2,"Ion",kion+20040);
    pdgDB->AddParticle("HE3","HE3",3*kAu2Gev+14.931e-3,kFALSE,
		       0,2,"Ion",kion+20030);
  }
  AliKalmanTrack::SetConvConst(1); 
}

ZAliLoad:: ZAliLoad(const Text_t* n, const Text_t* t) :
  ZNode(n,t),
  mOpMutex(GMutex::recursive)
{
  _init();
}

/**************************************************************************/

void ZAliLoad::SetupDataSource(Bool_t use_aliroot)
{
  static const string _eh("ZAliLoad::SetupDataSource ");

  OpMutexHolder omh(this, "SetupDataSource");

  ResetEvent();

  if(mDataDir == "")
    SetDataDir(".");

  if(use_aliroot == false) {

    string vsd_file (GForm("%s/%s", mDataDir.Data(), mVSDName.Data()));

    if(gSystem->AccessPathName(vsd_file.c_str(), kReadPermission) == false) {
      // Read OK!
      mFile      = new TFile(vsd_file.c_str());
      mDirectory = (TDirectory*) mFile->Get(GForm("Event%d", mEvent));
      if(!mDirectory)
	throw(_eh + GForm("event directory '%d' not found.", mEvent));
    } else {
      use_aliroot = true; 
      printf("VSD data does not exist, falling back to AliRunLoader.\n");
    }
  }

 
  if(use_aliroot && pRunLoader == 0){
    printf("Start importing AliRunLoader from dir %s\n", mDataDir.Data());

    string galice_file (GForm("%s/galice.root", mDataDir.Data()));
    printf("END importing AliRunLoader from dir %s\n", mDataDir.Data());

    // GTime time(GTime::I_Now);
    if(gSystem->AccessPathName(galice_file.c_str(), kReadPermission)) {
      // CAN'T Read
      throw(_eh + "Can not access file '" + galice_file + "'.");
    }
    pRunLoader = AliRunLoader::Open(galice_file.c_str());
    if(pRunLoader == 0)
      throw(_eh + "AliRunLoader::Open failed.");
    if(pRunLoader->GetEvent(mEvent) != 0)
      throw(_eh + GForm("event '%d' not found.", mEvent));
    pRunLoader->LoadgAlice();
    pRunLoader->LoadHeader();
    pRunLoader->LoadKinematics();
    //printf("End load KINEE \n");
    pRunLoader->LoadHits();
    //printf("End load HITTS \n");
    pRunLoader->LoadTrackRefs();
    //printf("End load Tr \n");
    //    GTime delta = time.TimeUntilNow();
    // printf("AliRunLoader::Open took %lf sec\n", delta.ToDouble());

    GledNS::PushFD();
    gDirectory = 0;
    mDirectory = new TDirectory(GForm("Event%d", mEvent), "RECREATE");
    GledNS::PopFD();
  }
  SetupEvent(use_aliroot);
}


void ZAliLoad::SetupEvent(Bool_t use_aliroot)
{
  if (use_aliroot) {
    LoadKinematics();
    LoadHits();
    LoadClusters();
    LoadRecTracks();
  } else {
    // printf("Importing kinematics \n");
    mTreeK = (TTree*) mDirectory->Get("Kinematics");
    if(mTreeK == 0) {
      printf("ERROR importing kinematics  \n");
    }else {
      mTreeK->SetBranchAddress("P", &mpP);
    }
    // printf("Importing hits TreeH %p \n", mTreeH);  
    mTreeH =  (TTree*) mDirectory->Get("Hits");
    if (mTreeH == 0){
      printf("ERROR importing hits\n");
    } else {
      mTreeH->SetBranchAddress("H", &mpH);
    }

    // printf("Importing reconstructed clusters. \n");
    mTreeC =  (TTree*) mDirectory->Get("Clusters");
    if (mTreeC == 0){
      printf("ERROR importing Clusters\n");
    } else {
      mTreeC->SetBranchAddress("C", &mpC);
    }

    // printf("Importing reconstructed tracks. \n");
    mTreeR =  (TTree*) mDirectory->Get("RecTracks");
    if (mTreeR == 0){
      printf("ERROR importing RecTracks\n");
    } else {
      mTreeR->SetBranchAddress("R", &mpR);
    }
  }
}

/**************************************************************************/

void ZAliLoad::Write()
{
  OpMutexHolder omh(this, "Write");

  string vsd_file (GForm("%s/%s", mDataDir.Data(), mVSDName.Data()));

  GledNS::PushFD();

  TFile* GAf = new TFile(GForm("%s/galice.root", mDataDir.Data()));
  AliTPCParam* par = (AliTPCParam *)GAf->Get("75x40_100x60_150x60");
  GAf->Close();
  delete GAf;

  TFile* vsd = new TFile(vsd_file.c_str(), "RECREATE",
			 "ALICE VisualizationDataSummary");
  gDirectory->cd();
  par->SetName("TPCParameter");
  par->Write();

  TDirectory* d = new TDirectory(mDirectory->GetName(), mDirectory->GetTitle());
  d->cd();
  if (mTreeK) {
    printf("write mTreeK %p\n", mTreeK);
    mTreeK->Write();
  }
  if (mTreeH) { 
    printf("write mTreeH %p\n", mTreeH);
    mTreeH->Write();
  }

  if (mTreeC){
    printf("write mTreeC %p\n", mTreeC);
    mTreeC->Write();
  }

  if (mTreeR) { 
    printf("write mTreeR %p\n", mTreeR);
    mTreeR->Write();
  }

  vsd->Write();
  vsd->Close();
  delete vsd;

  GledNS::PopFD();
}

/**************************************************************************/

void ZAliLoad::ResetEvent()
{
  OpMutexHolder omh(this, "ResetEvent");

  RemoveLensesViaQueen(true);

  delete mTreeK;      mTreeK      = 0;
  delete mTreeH;      mTreeH      = 0;
  delete mTreeTR;     mTreeTR     = 0;
  delete mTreeC;      mTreeC      = 0;
  delete mTreeR;      mTreeR      = 0;
  delete mTPCDigInfo; mTPCDigInfo = 0;

  if (mFile) {
    mFile->Close();
    delete mFile;
    mFile = 0; mDirectory = 0;
  }

  if (pRunLoader) {
    pRunLoader->UnloadAll();
    delete pRunLoader;
    if(gAlice){
      printf("Clear:: gAlice \n");
      delete gAlice; gAlice = 0; //!!!! dont know what it is used for
    }
    printf("Clear %p pRunLoader \n", pRunLoader);
    pRunLoader = 0;
  }
}

/**************************************************************************/
// Kinematics
/**************************************************************************/

void ZAliLoad::LoadKinematics()
{
  OpMutexHolder omh(this, "LoadKinematics");

  if(pRunLoader == 0) {
    printf("AliRunLoader not imported 'n");
    return;
  }
  mTreeK = new TTree("Kinematics", "Sorted TParticles as in Alistack");
  mDirectory->Add(mTreeK);
 
  TTree* treek = pRunLoader->TreeK();
  TParticle tp, *_tp = &tp;
  treek->SetBranchAddress("Particles", &_tp);
  TTreeQuery evl;
  Int_t nprimary = evl.Select(treek, "fMother[0]==-1");

  Int_t nentries = (Int_t)treek->GetEntries();
  vector<MCParticle>  vmc(nentries);
  for (Int_t idx=0; idx<nentries; idx++) {
    // index to entry
    Int_t ent = (idx < nprimary) ?
      idx + nentries - nprimary :
      idx - nprimary;

    treek->GetEntry(ent);
    //printf("Import Kinematics %s \n",tp.GetName());
    vmc[idx] = tp;
    vmc[idx].SetLabel(idx);
  }

  // read track refrences 
  TTree* mTreeTR =  pRunLoader->TreeTR();

  TClonesArray* RunArrayTR = 0; // new TClonesArray("AliTrackReference", 10);
  mTreeTR->SetBranchAddress("AliRun", &RunArrayTR);

  Int_t nPrimaries = (Int_t) mTreeTR->GetEntries();
  for (Int_t iPrimPart = 0; iPrimPart<nPrimaries; iPrimPart++) {
    mTreeTR->GetEntry(iPrimPart);
    for (Int_t iTrackRef = 0; iTrackRef < RunArrayTR->GetEntriesFast(); iTrackRef++) {
      AliTrackReference *trackRef = (AliTrackReference*)RunArrayTR->At(iTrackRef); 
      Int_t track = trackRef->GetTrack();
      if(track < treek->GetEntries()){
	vmc[track].SetEvaID(iPrimPart);
	MCParticle& mcp = vmc[track];

	if(trackRef->TestBit(kNotDeleted)) {
	  mcp.SetDecayed(true);
	  mcp.fDt=trackRef->GetTime();
	  mcp.fDx=trackRef->X(); mcp.fDy=trackRef->Y(); mcp.fDz=trackRef->Z();
	  mcp.fDPx=trackRef->Px(); mcp.fDPy=trackRef->Py(); mcp.fDPz=trackRef->Pz();
	  // if(mcp.GetPDG()->Stable()) printf("WARNING import TR %s track %d stable , but decayed \n", mcp.GetName(), track);
	  if(mcp.GetPdgCode() == 11)  mcp.SetDecayed(false); // a bug in TreeTR
	}
      } 
    }
  }

  mTreeK->Branch("P", "MCParticle",  &mpP, 512*1024, 99);
  for(vector<MCParticle>::iterator k=vmc.begin(); k!=vmc.end(); ++k) {
    MCParticle& mcp = *k;
    mP= mcp;
    // if(mP.bDecayed) printf("%s decayed \n",mP.GetName());
    mTreeK->Fill();   
  }
}

/**************************************************************************/

MCParticle* ZAliLoad::Particle(Int_t i)
{
  static const string _eh("ZAliLoad::Particle ");
  if(mTreeK == 0) 
    throw (_eh + "kinematics not available.");
  
  mTreeK->GetEntry(i);
  MCParticle* p = new MCParticle(mP); 
  return p;
}

ZParticle* ZAliLoad::create_particle_with_parents(Int_t label, Int_t depth)
{
  lpZATrack_t plist;
  MCParticle* p;
  ZParticle* zp; 

  Int_t idx = label;
  while(depth != 0){
    p= Particle(idx);
    zp = new ZParticle(p); 
    mQueen->CheckIn(zp); 
    plist.push_front(zp);
    idx = p->GetMother(0);

    if(idx == -1) break;
    depth--;
  }

  lpZATrack_i i = plist.begin();
  ZParticle* base = *i;
  // printf("retun base %s \n", base->GetName());
  plist.pop_front();
  ZParticle* holder = base;

  for(lpZATrack_i k=plist.begin(); k!=plist.end(); ++k) {
    zp = *k;
    // printf("Add %s to holder %s \n", zp->GetName(), holder->GetName());
    holder->Add(zp);
    holder = zp; 
  }
  // printf("retun base %s \n", base->GetName());
  return base;
}


void ZAliLoad::PrintTreeK()
{
  MCParticle*  m_P = &mP;
  Int_t nentries = (Int_t)mTreeK->GetEntries();
  printf("<< print treek  %d entries in branch  \n",nentries);

  for (Int_t i=0;i<nentries;i++) {
    mTreeK->GetEntry(i);
    printf("%4d %4d >> %s p(%f) PGD(%d)  \n", i, m_P->GetMother(0), 
	   m_P->GetName(), m_P->P(), m_P->GetPdgCode());
  }
}

/**************************************************************************/

void ZAliLoad::SelectParticles(ZNode* holder, const Text_t* selection,
			       Bool_t import_daughters)
{
  static const string _eh("ZAliLoad::ImportParticlesWithSelection ");

  OpMutexHolder omh(this, "SelectParticles");

  if(mTreeK == 0) 
    throw (_eh + "kinematics not available.");
  
  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mParticleSelection.Data();

  TTreeQuery evl;
  Int_t n = evl.Select(mTreeK, selection);
  // printf("%d entries in selection '%s'.\n", n,  selection);

  bool add_holder_p = false;
  if(holder == 0) {
    holder = new ZNode("Particles", selection);
    mQueen->CheckIn(holder);
    add_holder_p = true;
  }

  if(n > 0) {
    for (Int_t i=0; i<n; i++){
      Int_t label = evl.GetEntry(i);
      mTreeK->GetEntry(label);
      MCParticle* p = new MCParticle(*mpP); 
      ZParticle* zp = new ZParticle(p, GForm("%d %s", label, p->GetName()));
      mQueen->CheckIn(zp);
      if(import_daughters) zp->ImportDaughtersRec(this);
      holder->WriteLock();
      holder->Add(zp);
      holder->WriteUnlock();
    }
  }

  if(add_holder_p) Add(holder);
}

/**************************************************************************/

namespace {

  struct Detector 
  {
    const char* name;
    const char* hitbranch;
    unsigned char detidx;
  };

  Detector detects[] = {
    { "TPC", "AliTPCTrackHitsV2", 1 },
    { "ITS", "AliITShit", 0 },
    { "TRD", "AliTRDhit", 2 },
    { "TOF", "AliTOFhit", 3 },
    { "RICH", "AliRICHhit", 4 },
    { 0 }
  };

}

/*************************************************************************/

void ZAliLoad::LoadHits()
{
  static const string _eh("ZAliLoad::LoadHits ");

  OpMutexHolder omh(this, "LoadHits");

  if(pRunLoader == 0)
    throw(_eh + "AliRunLoader not available.");

  if(mTreeH != 0)
    throw(_eh + "hits already loaded.");

  mTreeH =  new TTree("Hits", "Combined detector hits.");
  // Hit *_H=&mH; //!!!!!!
  Hit::Class()->IgnoreTObjectStreamer(true);
  mTreeH->Branch("H", "Hit",  &mpH,128*1024,1);
  mDirectory->Add(mTreeH);
 

  int l=0;
  // load hits from the rest of detectors
  while(detects[l].name != 0) {
    Detector& det = detects[l++];

    switch(det.detidx) {
    case 1: { 
      Int_t count = 0;
      TTree* treeh = pRunLoader->GetTreeH(det.name, false);
      AliTPCTrackHitsV2 hv2, *_hv2=&hv2; 
      Float_t x=0,y=0,z=0, x1,y1,z1;
      treeh->SetBranchAddress("TPC2", &_hv2);
      Int_t np = treeh->GetEntries();
      for(Int_t i=0; i<np; i++){
	treeh->GetEntry(i);
	Int_t eva_idx = np -i -1;
	if (hv2.First() == 0) continue;
	do {
	  AliHit* ah = hv2.GetHit();
	  x1=ah->X();y1=ah->Y();z1=ah->Z();
	  if((x-x1)*(x-x1)+(y-y1)*(y-y1)+(z-z1)*(z-z1) > 4){
	    mH.fDetID=det.detidx;
	    mH.fEvaID=eva_idx;
	    mH.fLabel=ah->Track();
	    mH.x=x1; mH.y=y1; mH.z=z1;
	    mTreeH->Fill();
	    x=x1;y=y1;z=z1;
	    count++;
	  }
	} while (hv2.Next());
      }
      printf("%d entries in TPChits \n",count);
      break;
    }
    default: {
      TTree* treeh = pRunLoader->GetTreeH(det.name, false);
      TClonesArray *arr = new TClonesArray(det.hitbranch);
      treeh->SetBranchAddress(det.name,&arr);
      Int_t np = treeh->GetEntries();
      // in TreeH files hits are grouped in clones arrays
      // each eva particle has its own clone array 
      for ( Int_t i=0; i<np; i++) {
	treeh->GetEntry(i);
	Int_t eva_idx = np -i -1;
	Int_t nh=arr->GetEntriesFast();
	// printf("%d entry %d hits for primary %d \n", i, nh, eva_idx);
	for (Int_t j=0; j<nh; j++) {
	  AliHit* ali_hit = (AliHit*)arr->UncheckedAt(j);
	  mH.fEvaID=eva_idx;
	  mH.fDetID=det.detidx;
	  mH.fLabel=ali_hit->GetTrack();
	  mH.x=ali_hit->X(); mH.y=ali_hit->Y(); mH.z=ali_hit->Z();
	  // mH.Dump();
	  mTreeH->Fill(); 
	}
      }
      break;
    } // end default 
    } // end switch
  } // end whi/le
}

/**************************************************************************/

void ZAliLoad::SelectHits(HitContainer* holder, const char* selection)
{
  static const string _eh("ZAliLoad::ImportHitsWithSelection ");

  OpMutexHolder omh(this, "SelectHits");

  if(mTreeH == 0) 
    throw (_eh + "hits not available.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mHitSelection.Data();

  TTreeQuery evl;
  Int_t n = evl.Select(mTreeH, selection);
  // printf("ImportHitsWithSelection %d entries for selection %s\n", n, selection);
  
  bool add_holder_p = false;
  if(n > 0) {
    if( holder == 0 ) {
      holder = new HitContainer(GForm("Hits %s", selection));
      mQueen->CheckIn(holder);
      add_holder_p = true;
    }

    holder->Reset(n);
    for(Int_t i=0; i<n; i++) {
      const Int_t entry = evl.GetEntry(i);
      mTreeH->GetEntry(entry);
      holder->SetPoint(i, entry, &mpH->x);
    }
  } else {
    throw(_eh + "no hits matching selection.");
  }
  if(add_holder_p)
    Add(holder);
}

/**************************************************************************/
// Clusters
/**************************************************************************/

void ZAliLoad::LoadClusters()
{
  static const string _eh("ZAliLoad::LoadClusters ");

  OpMutexHolder omh(this, "LoadClusters");

  if(pRunLoader == 0)
    throw(_eh + "AliRunLoader not available.");

  if(mTreeC != 0)
    throw(_eh + "clusters already loaded.");

  mDirectory->cd();
  mTreeC =  new TTree("Clusters", "rec clusters");
  mTreeC->Branch("C", "Hit",  &mpC,128*1024,1);
  mDirectory->Add(mTreeC);

  LoadTPCClusters();
}

void ZAliLoad::SelectClusters(HitContainer* holder, const char* selection)
{
  static const string _eh("ZAliLoad::SelectClusters ");

  OpMutexHolder omh(this, "SelectClusters");

  if(mTreeC == 0) 
    throw (_eh + "clusters not available.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mClusterSelection.Data();

  TTreeQuery evl;
  Int_t n = evl.Select(mTreeC, selection);
  
  bool add_holder_p = false;
  if(n > 0) {
    if( holder == 0 ) {
      holder = new HitContainer(GForm("Clusers %s", selection));
      holder->SetColor(1.,1.,0.,1.);
      mQueen->CheckIn(holder);
      add_holder_p = true;
    }

    holder->Reset(n);
    for(Int_t i=0; i<n; i++) {
      const Int_t entry = evl.GetEntry(i);
      mTreeC->GetEntry(entry);
      holder->SetPoint(i, entry, &mpC->x);
    }
  } else {
    throw(_eh + "no hits matching selection.");
  }
  if(add_holder_p)
    Add(holder);
}

/**************************************************************************/
// ESD
/**************************************************************************/

void ZAliLoad::LoadRecTracks()
{
  static const string _eh("ZAliLoad::LoadRecTracks ");

  OpMutexHolder omh(this, "LoadRecTracks");

  if(pRunLoader == 0)
    throw(_eh + "AliRunLoader not available.");

  if(mTreeR != 0)
    throw(_eh + "tracks already loaded.");


  mDirectory->cd();
  mTreeR =  new TTree("RecTracks", "rec tracks");
  mTreeR->Branch("R", "ESDTrack",  &mpR,128*1024,1);
  mDirectory->Add(mTreeR);
 
  TFile f(GForm("%s/AliESDs.root", mDataDir.Data()));
  if(!f.IsOpen()){
    printf("no AliESDs.root file\n");
    return;
    // throw(_eh + "no AliESDs.root file\n");
  }

  TTree* tree = (TTree*) f.Get("esdTree");
  if (tree == 0) 
    throw(_eh + "no esdTree\n");

 
  AliESD *fEvent=0;  
  tree->SetBranchAddress("ESD", &fEvent);
  tree->GetEntry(0); 

 
  // reconstructed tracks
  AliESDtrack* esd_t;
  for (Int_t n =0; n< fEvent->GetNumberOfTracks();n++){
    esd_t = fEvent->GetTrack(n);
    esd_t->GetXYZ(mpR->fV);
    esd_t->GetPxPyPz(mpR->fP);
    mpR->fSign = esd_t->GetSign();
    mpR->fLabel = esd_t->GetLabel();
    mTreeR->Fill();
  }

  /*
  // V0
  ZNode* vh = new ZNode("V0Tracks");
  mQueen->CheckIn(vh);
  Add(vh);
  AliESDv0* v0;
  for (Int_t n =0; n< fEvent->GetNumberOfV0s();n++){
  printf("Importing V0 of %d\n",fEvent->GetNumberOfV0s() );
  v0 = fEvent->GetV0(n);
  V0* vt = new  V0(v0);
  mQueen->CheckIn(vt);
  vh->Add(vt);
  }
  */
}


void ZAliLoad::SelectRecTracks(ZNode* holder, const Text_t* selection)
{
  static const string _eh("ZAliLoad::SelectRecTracks ");

  OpMutexHolder omh(this, "SelectRecTracks");

  if(mTreeR == 0) 
    throw (_eh + "reconstructed tracks not available.");
  
  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mRecTrackSelection.Data();

  TTreeQuery evl;
  Int_t n = evl.Select(mTreeR, selection);
  // printf("%d entries in selection %s \n", n,  selection);

  bool add_holder_p = false;
  if(holder == 0) {
    holder = new ZNode("RecTracks", selection);
    mQueen->CheckIn(holder);
    add_holder_p = true;
  }

  if(n > 0) {
    for (Int_t i=0; i<n; i++){
      Int_t label = evl.GetEntry(i);
      mTreeR->GetEntry(label);
      // printf("Importing track %s \n", mpR->GetName());
      ESDTrack* et = new ESDTrack(*mpR);
      RecTrack* t = new RecTrack(et); 
      mQueen->CheckIn(t);
      holder->WriteLock();
      holder->Add(t);
      holder->WriteUnlock();
    }
  }
  if(add_holder_p) Add(holder);
}

void ZAliLoad::SelectRecTracksWithKine(ZNode* holder, const Text_t* selection, Int_t depth)
{
  static const string _eh("ZAliLoad::SelectRecTracksWithKine ");

  OpMutexHolder omh(this, "SelectRecTracksWithKine");

  if(mTreeK == 0) 
    throw (_eh + "reconstructed tracks not available.");
  
  bool add_holder_p = false;
  if(holder == 0) {
    holder = new ZNode("RecTracksWithKine", selection);
    mQueen->CheckIn(holder);
    add_holder_p = true;
  }

  // import ESD tracks
  ZNode* esd_holder = new ZNode("ESD", selection);
  mQueen->CheckIn(esd_holder);
  SelectRecTracks(esd_holder,selection);  
  holder->Add(esd_holder);



  ZNode* k_holder = new ZNode("Kine", selection);
  mQueen->CheckIn(k_holder);
  holder->Add(k_holder);
  // loop through the list of reconstracted track and import particle
  // with same label
  RecTrack* rt;
  ZParticle* zp;
  lpZGlass_i i, end;
  esd_holder->BeginIteration(i, end);
  while(i != end) {
    if((rt = dynamic_cast<RecTrack*>(*i))) {
      if(depth == 0){
	MCParticle* mcp = Particle(rt->GetESD()->GetLabel());
	zp = new ZParticle(mcp);
      }
      else {
        zp = create_particle_with_parents(rt->GetESD()->GetLabel(), depth);
      }
      mQueen->CheckIn(zp);
      k_holder->Add(zp);
    }
    ++i;
  }
  esd_holder->EndIteration();


  if(add_holder_p) Add(holder);
}

/**************************************************************************/
// TPC digits
/**************************************************************************/

TPCSegment* ZAliLoad::ShowTPCSegment(Int_t segment_id, ZNode* holder)
{
  static const string _eh("ZAliLoad::ShowTPCSegment ");

  if (mTPCDigInfo == 0) {
    AliTPCParam *par  = 0;
    TTree       *tree = 0;
    if(pRunLoader) {
      pRunLoader->CdGAFile();
      par = (AliTPCParam *)gDirectory->Get("75x40_100x60_150x60");
      pRunLoader->LoadDigits("TPC");
      tree = pRunLoader->GetTreeD("TPC", false);
    } else {
      TFile f(GForm("%s/%s", mDataDir.Data(), mVSDName.Data()));
      if(!f.IsOpen())
	throw(_eh + GForm("can not open '%s'.", mVSDName.Data()));
      par = (AliTPCParam *)f.Get("TPCParameter");
      f.Close();

      //gDirectory->cd();
      TFile* f2 = TFile::Open(GForm("%s/TPC.Digits.root", mDataDir.Data()));
      if(f2 == 0)
	throw(_eh + "can not open TPC.Digits.root.");
      //gDirectory->cd();
      TDirectory* d = (TDirectory*)f2->Get(GForm("Event%d",mEvent));
      //  printf("file %p dir %p\n",f2,d);
      //d->cd();
      tree = (TTree*)d->Get("TreeD");
    }
    mTPCDigInfo = new TPCDigitsInfo();
    mTPCDigInfo->SetData(par, tree);
  }

  Int_t down = mTPCDigInfo->mSegEnt[segment_id];
  Int_t up   = mTPCDigInfo->mSegEnt[segment_id + 36];
  if(down == -1 && up == -1){
    printf("No digits inner %d outer %d segment.\n", segment_id, segment_id + 36);
    // return 0;
  }

  TPCSegment* tpc = new TPCSegment();
  tpc->SetSegment(segment_id);
  tpc->SetDigInfo(mTPCDigInfo);
  tpc->RotateLF(1,2, - TMath::Pi()/2);
  tpc->RotateLF(1,2, + (segment_id + 0.5)*20*TMath::Pi()/180);
  tpc->RotateLF(1,3, TMath::Pi());
  if(segment_id < 18) {
    tpc->Move3(0,0,  mTPCDigInfo->mParameter->GetZLength());
  } else {
    tpc->Move3(0,0, -mTPCDigInfo->mParameter->GetZLength());
  }
 
  mQueen->CheckIn(tpc);
  if (holder == 0) holder = this;
  holder->Add(tpc);
  return tpc;
}

void ZAliLoad::ShowTPCPlate(Int_t side)
{
  ZGlass* cfg;
  try {
    cfg = mQueen->FindLensByPath("Etc/Alice/TPC_RM_list");
  }
  catch(...) {}

  if (side == 0 || side == -1) {
    ZNode* holder = new ZNode(GForm("TPC plate %d", 0));
    mQueen->CheckIn(holder);
    for (int i=0; i < 18; i++) ShowTPCSegment(i, holder);
    Add(holder);
    if(cfg) holder->SetRnrMod(cfg);
  } 
  if ( side == 1 || side == -1) {
    ZNode* holder = new ZNode(GForm("TPC plate %d", 1));
    mQueen->CheckIn(holder);
    for (int i=18; i < 2*18; i++) ShowTPCSegment(i, holder);
    Add(holder);
    if(cfg) holder->SetRnrMod(cfg);
  }
}

/**************************************************************************/

void ZAliLoad::LoadTPCClusters()
{
  static const string _eh("ZAliLoad::LoadTPCClusters ");

  auto_ptr<TFile> f 
    ( TFile::Open(GForm("%s/TPC.RecPoints.root", mDataDir.Data())) );
  if(!f.get())
    throw(_eh + "can not open 'TPC.RecPoints.root' file.");

  auto_ptr<TDirectory> d
    ( (TDirectory*) f->Get(GForm("Event%d",mEvent)) );
  if(!d.get())
    throw(_eh + GForm("event directory '%d' not found.", mEvent));

  auto_ptr<TTree> tree( (TTree*) d->Get("TreeR") );
  if(!tree.get())
    throw(_eh + "'TreeR' not found.");

  auto_ptr<AliTPCParam> par( get_tpc_param(_eh) );

  AliTPCClustersRow  clrow, *_clrow=&clrow;
  AliTPCclusterMI *cl;
  _clrow->SetClass("AliTPCclusterMI");
  tree->SetBranchAddress("Segment", &_clrow);

  // count clusters
  Int_t nClusters = 0;
  Int_t n_ent = tree->GetEntries();
  for (Int_t n=0; n<n_ent; n++) {
    tree->GetEntry(n);
    nClusters += _clrow->GetArray()->GetEntriesFast();
  }

  // calculate xyz for a cluster and add it to container 
  Double_t x,y,z;
  Float_t cs, sn, tmp;

  for (Int_t n=0; n<tree->GetEntries(); n++) {
    tree->GetEntry(n);
    Int_t ncl=_clrow->GetArray()->GetEntriesFast();
    if(ncl > 0){
      Int_t sec,row;
      par->AdjustSectorRow(_clrow->GetID(),sec,row);    
      while (ncl--) {
	if(_clrow->GetArray()){
	  cl=new AliTPCclusterMI(*(AliTPCclusterMI*)_clrow->GetArray()->UncheckedAt(ncl));
	  x=par->GetPadRowRadii(sec,row); y=cl->GetY(); z=cl->GetZ();
	  par->AdjustCosSin(sec,cs,sn);
	  tmp = x*cs-y*sn; y= x*sn+y*cs; x=tmp; 
	  Int_t trackId = cl->GetLabel(0);
	  mC.fDetID=1;
	  mC.fLabel = trackId;
          mC.x = x;
          mC.y = y;
          mC.z = z;
          mTreeC->Fill();
	}
      }   
    }
  }

}

/**************************************************************************/
/**************************************************************************/
// Protected methods
/**************************************************************************/
/**************************************************************************/

AliTPCParam* ZAliLoad::get_tpc_param(const string& eh)
{
  auto_ptr<TFile> fp( TFile::Open(GForm("%s/galice.root", mDataDir.Data())) );
  if(!fp.get())
    throw(eh + "can not open 'galice.root' file.");
  AliTPCParam* par = (AliTPCParam *) fp->Get("75x40_100x60_150x60");
  if(!par)
    throw(eh + "TPC data not found.");
  return par;
}
