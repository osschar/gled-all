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
#include <AliESDV0MI.h>
#include <AliTPCclusterMI.h>
#include <AliTPCClustersRow.h>

typedef list<MCTrack*>           lpZATrack_t;
typedef list<MCTrack*>::iterator lpZATrack_i;

map<Int_t, GenInfo*> gimap;

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
      TString new_op = mOldOp == "<idle>" ? "" : mOldOp + "::";
      mAli->SetOperation(new_op + op);
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
  mTreeV0 = 0;
  mTreeGI = 0;
  mpP  = &mP;
  mpH  = &mH;
  mpC  = &mC;
  mpR  = &mR;
  mpV0 = &mV0;
  mpGI = &mGI;
  mTPCDigInfo = 0;
  mGIIStyle = 0;

  mDataDir  = ".";
  mEvent    = 0;
  mKineType = KT_Standard;

  mVSDName = "AliVSD.root";
  mFile    = 0; mDirectory = 0;

  mOperation = "<idle>";

  mParticleSelection = "fMother[0] == -1";
  mHitSelection      = "fDetID < 5";
  mClusterSelection  = mHitSelection;
  mRecSelection      = "fLabel >= 0";
  mV0Selection       = "fStatus == 100";
  mGISelection       = "bR == 1";
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
    try {
      open_vsd();
    }
    catch(string exc) {
      ISwarn(_eh + exc + " Falling back to AliRunLoader.");
      use_aliroot = true;
    }
  }

  if(use_aliroot) {
    printf("Start importing AliRunLoader from dir %s\n", mDataDir.Data());
    // GTime time(GTime::I_Now);

    string galice_file (GForm("%s/galice.root", mDataDir.Data()));

    if(gSystem->AccessPathName(galice_file.c_str(), kReadPermission)) {
      throw(_eh + "Can not read file '" + galice_file + "'.");
    }
    pRunLoader = AliRunLoader::Open(galice_file.c_str());
    if(pRunLoader == 0)
      throw(_eh + "AliRunLoader::Open failed.");
    if(pRunLoader->GetEvent(mEvent) != 0)
      throw(_eh + GForm("event '%d' not found.", mEvent));

    pRunLoader->LoadgAlice();
    pRunLoader->LoadHeader();
    pRunLoader->LoadKinematics();
    pRunLoader->LoadHits();
    pRunLoader->LoadTrackRefs();

    CreateVSD();

    open_vsd();

    //GTime delta = time.TimeUntilNow();
    printf("END importing AliRunLoader from dir '%s'.\n", mDataDir.Data());
  }

  SetupEvent();
}


void ZAliLoad::SetupEvent()
{
  static const string _eh("ZAliLoad::SetupEvent ");
 

  printf("Reading kinematics.\n");
  mTreeK = (TTree*) mDirectory->Get("Kinematics");
  if (mTreeK == 0) {
    printf("%s Kinematics not available in directory %s.\n", 
	   _eh.c_str(), mDirectory->GetName());
  } else {
    mTreeK->SetBranchAddress("P", &mpP);
  }
  printf("Reading hits.\n");  
  mTreeH = (TTree*) mDirectory->Get("Hits");
  if (mTreeH == 0) {
    printf("%s Hits not available in directory %s.\n", 
	   _eh.c_str(), mDirectory->GetName());
  } else {
    mTreeH->SetBranchAddress("H", &mpH);
  }

  printf("Reading clusters.\n");
  mTreeC = (TTree*) mDirectory->Get("Clusters");
  if (mTreeC == 0) {
    printf("%s Clusters not available in directory %s.\n", 
	   _eh.c_str(), mDirectory->GetName());
  } else {
    mTreeC->SetBranchAddress("C", &mpC);
  }


  printf("Reading reconstructed tracks.\n");
  mTreeR = (TTree*) mDirectory->Get("RecTracks");
  if (mTreeR == 0){ 
    printf("%s RecTracks not available in directory %s.\n", 
	   _eh.c_str(), mDirectory->GetName());
  } else {
    mTreeR->SetBranchAddress("R", &mpR);
  }

  printf("Reading V0 points. \n");
  mTreeV0 =  (TTree*) mDirectory->Get("V0");
  if (mTreeV0 == 0){
    printf("%s V0 not available in directory %s.\n", 
	   _eh.c_str(), mDirectory->GetName());
  } else {
    mTreeV0->SetBranchAddress("V0", &mpV0);
  }
    
  printf("Reading GenInfo.\n");
  mTreeGI = (TTree*)mDirectory->Get("GenInfo");
  if (mTreeGI == 0) {
    printf("%s GenInfo not available in directory %s.\n", 
	   _eh.c_str(), mDirectory->GetName());
  } else {
    mTreeGI->SetBranchAddress("GI", &mpGI);
    mTreeGI->SetBranchAddress("P.", &mpP);
    mTreeGI->SetBranchAddress("R.", &mpR);
  }
}

/**************************************************************************/

void ZAliLoad::open_vsd()
{
  // Opens VSD in READ mode. Throws exception if not found.

  string vsd_file (GForm("%s/%s", mDataDir.Data(), mVSDName.Data()));

  mFile = TFile::Open(vsd_file.c_str());
  if(!mFile)
    throw(string(GForm("can not open VSD file '%s'.", vsd_file.c_str())));

  mDirectory = (TDirectory*) mFile->Get(GForm("Event%d", mEvent));
  if(!mDirectory) {
    delete mFile; mFile = 0;
    throw(string(GForm("event directory 'Event%d' not found in VSD file '%s'.",
		       mEvent, vsd_file.c_str())));
  }
}

void ZAliLoad::close_vsd()
{
  delete mTreeK;      mTreeK      = 0;
  delete mTreeH;      mTreeH      = 0;
  delete mTreeTR;     mTreeTR     = 0;
  delete mTreeC;      mTreeC      = 0;
  delete mTreeR;      mTreeR      = 0;
  delete mTreeV0;     mTreeV0     = 0;
  delete mTreeGI;     mTreeGI     = 0;
  delete mTPCDigInfo; mTPCDigInfo = 0;

  if (mFile) {
    mFile->Close();
    delete mFile;
    mFile = 0; mDirectory = 0;
  }
}

void ZAliLoad::CreateVSD()
{
  static const string _eh("ZAliLoad::CreateVSD ");

  OpMutexHolder omh(this, "CreateVSD");

  string vsd_file (GForm("%s/%s", mDataDir.Data(), mVSDName.Data()));

  GledNS::PushFD();

  pRunLoader->CdGAFile();
  AliTPCParam* par = (AliTPCParam *) gDirectory->Get("75x40_100x60_150x60");

  // Single file per event
  mFile = new TFile(vsd_file.c_str(), "RECREATE", "ALICE VisualizationDataSummary");

  par->SetName("TPCParameter");
  par->Write();

  mDirectory = new TDirectory(GForm("Event%d", mEvent), "");

  ConvertKinematics();
  ConvertHits();
  ConvertClusters();
  ConvertRecTracks();
  ConvertV0();
  ConvertGenInfo();

  mFile->Write();

  close_vsd();

  GledNS::PopFD();
}

/**************************************************************************/

void ZAliLoad::ResetEvent()
{
  OpMutexHolder omh(this, "ResetEvent");

  RemoveLensesViaQueen(true);

  close_vsd();

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

void ZAliLoad::ConvertKinematics()
{
  static const string _eh("ZAliLoad::ConvertKinematics ");

  OpMutexHolder omh(this, "ConvertKinematics");

  if(pRunLoader == 0)
    throw(_eh + "AliRunLoader not imported.");

  mDirectory->cd();
  mTreeK = new TTree("Kinematics", "Sorted TParticles as in Alistack");
 
  TTree* treek = pRunLoader->TreeK();
  TParticle tp, *_tp = &tp;
  treek->SetBranchAddress("Particles", &_tp);

  Text_t* prim_selection = 0;
  switch(mKineType) {
  case KT_Standard:     prim_selection = "fMother[0] == -1"; break;
  case KT_ProtonProton: prim_selection = "fStatusCode > 0";  break;
  }
  TTreeQuery evl;
  Int_t nprimary = evl.Select(treek, prim_selection);

  Int_t nentries = (Int_t)treek->GetEntries();
  vector<MCParticle>  vmc(nentries);
  for (Int_t idx=0; idx<nentries; idx++) {
    // index to entry
    Int_t ent = (idx < nprimary) ?
      idx + nentries - nprimary :
      idx - nprimary;

    treek->GetEntry(ent);
    //printf("Convert Kinematics %s \n",tp.GetName());
    vmc[idx] = tp;
    vmc[idx].SetLabel(idx);
  }

  // read track refrences 
  TTree* mTreeTR =  pRunLoader->TreeTR();
  TClonesArray* RunArrayTR = 0;
  mTreeTR->SetBranchAddress("AliRun", &RunArrayTR);

  Int_t nPrimaries = (Int_t) mTreeTR->GetEntries();
  for (Int_t iPrimPart = 0; iPrimPart<nPrimaries; iPrimPart++) {
    // printf("START mTreeTR->GetEntry(%d) \n",iPrimPart);
    mTreeTR->GetEntry(iPrimPart);
    // printf("END mTreeTR->GetEntry(%d) \n",iPrimPart);
    
    for (Int_t iTrackRef = 0; iTrackRef < RunArrayTR->GetEntriesFast(); iTrackRef++) {
      AliTrackReference *trackRef = (AliTrackReference*)RunArrayTR->At(iTrackRef); 
      Int_t track = trackRef->GetTrack();
      if(track < nentries && track > 0){ 
	MCParticle& mcp = vmc[track];	
	if(trackRef->TestBit(kNotDeleted)) {
	  mcp.SetDecayed(true);
	  mcp.fDt=trackRef->GetTime();
	  mcp.fDx=trackRef->X(); mcp.fDy=trackRef->Y(); mcp.fDz=trackRef->Z();
	  mcp.fDPx=trackRef->Px(); mcp.fDPy=trackRef->Py(); mcp.fDPz=trackRef->Pz();
	  if(mcp.GetPdgCode() == 11)  mcp.SetDecayed(false); // a bug in TreeTR
	}
      }       
    }
  }

  mTreeK->Branch("P", "MCParticle",  &mpP, 512*1024, 99);
  for(vector<MCParticle>::iterator k=vmc.begin(); k!=vmc.end(); ++k) {
    MCParticle& mcp = *k;
    mP= mcp;

    TParticle* m  = &mcp;
    Int_t      mi = mcp.GetLabel();
    while( m->GetMother(0) != -1){
      mi = m->GetMother(0);
      m = &vmc[mi];
    }
    mP.SetEvaLabel(mi);
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
  static const string _eh("ZAliLoad::SelectParticles ");

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
      MCTrack* zp = new MCTrack(p, GForm("%d %s", label, p->GetName()));
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
// Hits
/**************************************************************************/

namespace {

  struct Detector 
  {
    const char*   name;
    const char*   hitbranch;
    unsigned char detidx;
  };

  Detector detects[] = {
    { "ITS",  "AliITShit",         0 },
    { "TPC",  "AliTPCTrackHitsV2", 1 },
    { "TRD",  "AliTRDhit",         2 },
    { "TOF",  "AliTOFhit",         3 },
    // { "RICH", "AliRICHhit",        4 },
    { 0 }
  };

}

/*************************************************************************/

void ZAliLoad::ConvertHits()
{
  static const string _eh("ZAliLoad::ConvertHits ");

  OpMutexHolder omh(this, "ConvertHits");

  if(pRunLoader == 0)
    throw(_eh + "AliRunLoader not available.");

  if(mTreeH != 0)
    throw(_eh + "hits already loaded.");

  mDirectory->cd();
  mTreeH =  new TTree("Hits", "Combined detector hits.");
  Hit::Class()->IgnoreTObjectStreamer(true);
  mTreeH->Branch("H", "Hit", &mpH, 512*1024, 1);
 
  map<Int_t, Int_t> hmap;

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
	  if((x-x1)*(x-x1)+(y-y1)*(y-y1)+(z-z1)*(z-z1) > 4) {
	    mH.fDetID    = det.detidx;
	    mH.fEvaLabel = eva_idx;
	    mH.fLabel    = ah->Track();
	    mH.x = x1; mH.y = y1; mH.z = z1;
	    Int_t xx = mTreeH->Fill();
	    if(xx != 21)
	      printf("Hit: %d %d %d %d\n", xx, mH.fLabel, mH.fEvaLabel, mH.fDetID);

	    hmap[mH.fLabel]++;
	    x=x1; y=y1; z=z1;
	    count++;
	  }
	} while (hv2.Next());
      }
      // printf("%d entries in TPChits \n",count);
      break;
    }
    default: {
      TTree* treeh = pRunLoader->GetTreeH(det.name, false);
      TClonesArray *arr = new TClonesArray(det.hitbranch);
      treeh->SetBranchAddress(det.name, &arr);
      Int_t np = treeh->GetEntries();
      // in TreeH files hits are grouped in clones arrays
      // each eva particle has its own clone array 
      for (Int_t i=0; i<np; i++) {
	treeh->GetEntry(i);
	Int_t eva_idx = np -i -1;
	Int_t nh=arr->GetEntriesFast();
	// printf("%d entry %d hits for primary %d \n", i, nh, eva_idx);
	for (Int_t j=0; j<nh; j++) {
	  AliHit* ali_hit = (AliHit*)arr->UncheckedAt(j);
	  mH.fEvaLabel = eva_idx;
	  mH.fDetID    = det.detidx;
	  mH.fLabel    = ali_hit->GetTrack();
	  mH.x=ali_hit->X(); mH.y=ali_hit->Y(); mH.z=ali_hit->Z();
	  hmap[mH.fLabel]++;
	  Int_t xx = mTreeH->Fill(); 
	  if(xx != 21)
	    printf("Hit: %d %d %d %d\n", xx, mH.fLabel, mH.fEvaLabel, mH.fDetID);
	}
      }
      delete arr;
      break;
    } // end default 
    } // end switch
  } // end while
  
  //set geninfo
  for(map<Int_t, Int_t>::iterator j=hmap.begin(); j!=hmap.end(); ++j) {
    get_geninfo(j->first)->Nh += j->second;
  }
  
}

/**************************************************************************/

void ZAliLoad::SelectHits(HitContainer* holder, const char* selection)
{
  static const string _eh("ZAliLoad::SelectHits ");

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

void ZAliLoad::ConvertClusters()
{
  static const string _eh("ZAliLoad::ConvertClusters ");

  OpMutexHolder omh(this, "ConvertClusters");

  if(pRunLoader == 0)
    throw(_eh + "AliRunLoader not available.");

  if(mTreeC != 0)
    throw(_eh + "clusters already loaded.");

  mDirectory->cd();
  mTreeC =  new TTree("Clusters", "rec clusters");
  mTreeC->Branch("C", "Hit", &mpC, 128*1024, 1);

  ConvertTPCClusters();
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

void ZAliLoad::ConvertRecTracks()
{
  static const string _eh("ZAliLoad::ConvertRecTracks ");

  OpMutexHolder omh(this, "ConvertRecTracks");

  if(pRunLoader == 0)
    throw(_eh + "AliRunLoader not available.");

  if(mTreeR != 0)
    throw(_eh + "tracks already loaded.");

  mDirectory->cd();
  mTreeR =  new TTree("RecTracks", "rec tracks");

  ESDParticle::Class()->IgnoreTObjectStreamer(true);
  mTreeR->Branch("R", "ESDParticle", &mpR, 512*1024,1);
 
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
  mTreeR->BuildIndex("fLabel");
 
}


void ZAliLoad::SelectRecTracks(ZNode* holder, const Text_t* selection)
{
  static const string _eh("ZAliLoad::SelectRecTracks ");

  OpMutexHolder omh(this, "SelectRecTracks");

  if(mTreeR == 0) 
    throw (_eh + "reconstructed tracks not available.");
  
  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mRecSelection.Data();

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
      ESDParticle* et = new ESDParticle(*mpR);
      RecTrack* t = new RecTrack(et); 
      mQueen->CheckIn(t);
      holder->WriteLock();
      holder->Add(t);
      holder->WriteUnlock();
    }
  }
  if(add_holder_p) Add(holder);
}


void ZAliLoad::ConvertV0()
{
  static const string _eh("ZAliLoad::LoadV0 ");

  OpMutexHolder omh(this, "ConvertV0");

  if(pRunLoader == 0)
    throw(_eh + "AliRunLoader not available.");

  if(mTreeV0 != 0)
    throw(_eh + "V0 already loaded.");

  mDirectory->cd();
  mTreeV0 =  new TTree("V0", "V0 points");

  ESDParticle::Class()->IgnoreTObjectStreamer(true);
  mTreeV0->Branch("V0", "V0", &mpV0, 512*1024,1);

  TFile f(GForm("%s/AliESDs.root", mDataDir.Data()));
  if(!f.IsOpen()){
    throw(_eh + "no AliESDs.root file\n");
  }

  TTree* tree = (TTree*) f.Get("esdTree");
  if (tree == 0) 
    throw(_eh + "no esdTree\n");

  AliESD *fEvent=0;  
  tree->SetBranchAddress("ESD", &fEvent);
  tree->GetEntry(0); 

  for (Int_t n =0; n< fEvent->GetNumberOfV0MIs(); n++) {
    AliESDV0MI* av = fEvent->GetV0MI(n);
    Double_t x,y,cos,sin; 

    mV0.fStatus = av->GetStatus();
    // distance to closest approach
    mV0.fVDCA[0] = av->GetXr(0); 
    mV0.fVDCA[1] = av->GetXr(1);
    mV0.fVDCA[2] = av->GetXr(2);
    // set birth vertex of neutral particle     
    av->GetXYZ(mV0.fV0[0],mV0.fV0[1],mV0.fV0[2]);

    // momentum and position of negative particle
    Double_t* pp = av->GetPMp();
    mV0.fPM[0]=pp[0];  mV0.fPM[1]=pp[1]; mV0.fPM[2]=pp[2];
    // read AliExternalTrackParam
    x = av->GetParamM()->X(); 
    y = av->GetParamM()->Y(); 
    cos = TMath::Cos( av->GetParamM()->Alpha());
    sin = TMath::Sin( av->GetParamM()->Alpha());
    mV0.fVM[0] = x*cos - y*sin;
    mV0.fVM[1] = x*sin + y*cos;
    mV0.fVM[2] = av->GetParamM()->Z();

    // momentum and position of positive particle
    Double_t* pm = av->GetPPp();
    mV0.fPP[0]=pm[0];  mV0.fPP[1]=pm[1]; mV0.fPP[2]=pm[2];
    x = av->GetParamP()->X();
    y = av->GetParamP()->Y();
    cos = TMath::Cos(av->GetParamP()->Alpha());
    sin = TMath::Sin(av->GetParamP()->Alpha());
    mV0.fVP[0] = x*cos - y*sin;
    mV0.fVP[1] = x*sin + y*cos;
    mV0.fVP[2] = av->GetParamP()->Z();

    // daughter indices
    mV0.fLabels[0] = av->GetLab(0);
    mV0.fLabels[1] = av->GetLab(1);

    mV0.fPDG = av->GetPdgCode();

    mTreeV0->Fill();
  }
}

void ZAliLoad::SelectV0(ZNode* holder, const Text_t* selection)
{
  static const string _eh("ZAliLoad::SelectV0 ");

  //  OpMutexHolder omh(this, "SelectV0 ");

  if(mTreeV0 == 0) 
    throw (_eh + "V0 tree not available.");
  
  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mV0Selection.Data();

  TTreeQuery evl;
  Int_t n = evl.Select(mTreeV0, selection);
  // printf("%d entries in selection %s \n", n,  selection);

  bool add_holder_p = false;
  if(holder == 0) {
    holder = new ZNode("V0", selection);
    mQueen->CheckIn(holder);
    add_holder_p = true;
  }

  if(n > 0) {
    for (Int_t i=0; i<n; i++){
      mTreeV0->GetEntry( evl.GetEntry(i));
      // nutral mother
      Double_t pp[3];
      pp[0]= mV0.fPP[0]+mV0.fPM[0];
      pp[1]= mV0.fPP[1]+mV0.fPM[1];
      pp[2]= mV0.fPP[2]+mV0.fPM[2];
      ESDParticle* v = new ESDParticle(&mV0.fVDCA[0], &pp[0],-1, 0);
      RecTrack* tV0 = new RecTrack(v); 
      tV0->SetName(GForm("V0 %d : %d {%d}", mV0.fLabels[0], mV0.fLabels[1], mV0.fPDG));
      mQueen->CheckIn(tV0);
      holder->Add(tV0);
      // minus daughter
      ESDParticle* m = new ESDParticle(&mV0.fVM[0], &mV0.fPM[0], mV0.fLabels[0], -1);
      RecTrack* tM = new RecTrack(m); 
      mQueen->CheckIn(tM);
      tV0->Add(tM);
      // plus daughter
      ESDParticle* p = new ESDParticle(&mV0.fVP[0], &mV0.fPP[0], mV0.fLabels[1], 1);
      RecTrack* tP = new RecTrack(p); 
      mQueen->CheckIn(tP);
      tV0->Add(tP);
    }
  }

  if(add_holder_p) Add(holder);
}

/**************************************************************************/
// GenInfo
/**************************************************************************/

void ZAliLoad::ConvertGenInfo()
{
  static const string _eh("ZAliLoad::ConvertGenInfo ");

  OpMutexHolder omh(this, "ConvertGenInfo");

  mDirectory->cd();
  mTreeGI = new TTree("GenInfo", "Objects prepared for cross querry");

  GenInfo::Class()->IgnoreTObjectStreamer(true);
  mTreeGI->Branch("GI", "GenInfo", &mpGI, 512*1024, 99);
  mTreeGI->Branch("P.", "MCParticle", &mpP);
  mTreeGI->Branch("R.", "ESDParticle",   &mpR);

  for(map<Int_t, GenInfo*>::iterator j=gimap.begin(); j!=gimap.end(); ++j) {
    mGI = *(j->second);
    mGI.fLabel = j->first;
    mTreeK->GetEntry(j->first);

    Int_t re = mTreeR->GetEntryNumberWithIndex(j->first);
    if(re != -1){
      mGI.bR = 1;
      mTreeR->GetEntry(re);
      // printf(">>> %d track with label %d", re, j->first);
      // printf(">>> R %d  Pz %f \n", mpR->fLabel,mpR->Pz() );
    }    
    mTreeGI->Fill();
  }
  gimap.clear();
}

void ZAliLoad::SelectGenInfo(ZNode* holder, const Text_t* selection)
{
  static const string _eh("ZAliLoad::SelectGenInfo ");

  OpMutexHolder omh(this, "SelectGenInfo");

  if(mTreeGI == 0)
    throw (_eh + "mTreeGI not available.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mGISelection.Data();

  TTreeQuery evl;
  Int_t nlabels = evl.Select(mTreeGI, selection);
  // printf("%d entries in selection %s \n", nlabels,  selection);


  if(holder == 0) {
    holder = new ZNode(GForm("GI %s", selection));
    mQueen->CheckIn(holder);
    Add(holder);
  }

  // holders for reconstructed and MC particles
  ZNode* mc_holder =  new ZNode(GForm("MC  %s", selection));
  ZNode* rec_holder =  new ZNode(GForm("Rec %s", selection));
  mQueen->CheckIn(mc_holder); mQueen->CheckIn(rec_holder);
  holder->Add(mc_holder); holder->Add(rec_holder);

  Int_t nc = 0, nh = 0;
  // Int_t labels[nlabels];
  set<Int_t> labels;

  for (Int_t i=0; i<nlabels; i++){
    mTreeGI->GetEntry(evl.GetEntry(i));

    labels.insert(mGI.fLabel);
    //    printf("import mgi %d  \n", mGI.fLabel );

    MCParticle* p = new MCParticle(mP); // check if gimap exists
    MCTrack* zp  = new MCTrack(p);
    mQueen->CheckIn(zp);
    mc_holder->Add(zp);
    
    if(mGI.bR == 1){
      // printf("Created rec track %d \n", mGI.fLabel);
      ESDParticle* t = new ESDParticle(mR);
      RecTrack* rt = new RecTrack(t);
      mQueen->CheckIn(rt);
      rec_holder->Add(rt);
    }
    nh += mpGI->Nh;
    nc += mpGI->Nc;
  }

  if(nh > 0) {
    HitContainer* hcont = new HitContainer(GForm("%d Hits", nh));
    hcont->Reset(nh);

    Int_t  all_hits   = mTreeH->GetEntries();
    Int_t  count      = 0;
    Int_t  last_label = -1;
    Bool_t store_p    = false;
    for(Int_t i=0; i<all_hits; ++i) {
      mTreeH->GetEntry(i);
      if(mH.fLabel != last_label) {
	store_p    = ( labels.find(mH.fLabel) != labels.end() );
	last_label = mH.fLabel;
      }
      if(store_p) {
	hcont->SetPoint(count, last_label, &mH.x);
	++count;
      }
    }

    mQueen->CheckIn(hcont);
    holder->Add(hcont);
  }
  
  // printf("Selecting clusters in GI select \n");
  if(nc > 0) {
    HitContainer* ccont = new HitContainer(GForm("%d Clusters", nc));
    ccont->Reset(nc);
    ccont->SetColor(1.,1.,0.,1.);

    Int_t  all_clus   = mTreeC->GetEntries();
    Int_t  count      = 0;
    Int_t  last_label = -1;
    Bool_t store_p    = false;
    for(Int_t i=0; i<all_clus; ++i) {
      mTreeC->GetEntry(i);
      if(mC.fLabel != last_label) {
	store_p    = ( labels.find(mC.fLabel) != labels.end() );
	last_label = mC.fLabel;
      }
      if(store_p) {
	ccont->SetPoint(count, last_label, &mC.x);
	++count;
      }
    }

    mQueen->CheckIn(ccont);
    holder->Add(ccont);
  }

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

  /*
    Int_t down = mTPCDigInfo->mSegEnt[segment_id];
    Int_t up   = mTPCDigInfo->mSegEnt[segment_id + 36];
 
    if(down == -1 && up == -1){
    printf("No digits inner %d outer %d segment.\n", segment_id, segment_id + 36);
    }
  */

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
  ZGlass* cfg = 0;
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

void ZAliLoad::ConvertTPCClusters()
{
  static const string _eh("ZAliLoad::ConvertTPCTPCClusters ");

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
  AliTPCclusterMI   *cl;
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
  map<Int_t, Int_t> cmap;

  for (Int_t n=0; n<tree->GetEntries(); n++) {
    tree->GetEntry(n);
    Int_t ncl = _clrow->GetArray()->GetEntriesFast();
    if(ncl > 0) {
      Int_t sec,row;
      par->AdjustSectorRow(_clrow->GetID(),sec,row);    
      while (ncl--) {
	if(_clrow->GetArray()) {
	  // cl = new AliTPCclusterMI(*(AliTPCclusterMI*)_clrow->GetArray()->UncheckedAt(ncl));
	  cl = (AliTPCclusterMI*)_clrow->GetArray()->UncheckedAt(ncl);
          if(cl->GetLabel(0) >= 0){
	    x = par->GetPadRowRadii(sec,row); y = cl->GetY(); z = cl->GetZ();
	    par->AdjustCosSin(sec,cs,sn);
	    tmp = x*cs-y*sn; y= x*sn+y*cs; x=tmp; 

	    mC.fDetID = 1;
	    mC.fLabel = cl->GetLabel(0);
	    mC.x      = x;
	    mC.y      = y;
	    mC.z      = z;

	    mTreeC->Fill();
	    cmap[mC.fLabel]++;
	  }
	}
      }
    }
  }
  //set geninfo
  for(map<Int_t, Int_t>::iterator j=cmap.begin(); j!=cmap.end(); ++j) {
    get_geninfo(j->first)->Nc += j->second;
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



GenInfo* ZAliLoad::get_geninfo(Int_t label)
{
  // printf("get_geninfo %d\n", label);
  GenInfo* gi;
  map<Int_t, GenInfo*>::iterator i = gimap.find(label);
  if(i == gimap.end()) {
    gi =  new GenInfo();
    gimap[label] = gi;
  } else {
    gi = i->second;
  }
  return gi;
}
