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

typedef list<ZParticle*>           lpZATrack_t;
typedef list<ZParticle*>::iterator lpZATrack_i;

ClassImp(ZAliLoad)

/**************************************************************************/

void ZAliLoad::_init()
{
  // *** Set all links to 0 ***
  mTreeK  = 0;
  mTreeH  = 0;
  mTreeTR = 0;
  mpP = &mP;
  mpH = &mH;
  mTPCDigInfo = 0;

  mDataDir = ".";
  mEvent   = 0;

  mVSDName = "aligled.root";
  mFile    = 0; mDirectory = 0;

  mParticleSelection = "GetMother(0) == -1";
  mHitSelection      = "det_ID < 5";

  pRunLoader = 0;

  // Pain:
  AliPDG::AddParticlesToPdgDataBase();
}

/**************************************************************************/

void ZAliLoad::SetupDataSource(Bool_t use_aliroot)
{
  static const string _eh("ZAliLoad::SetUpDataSource ");

  ResetEvent();

  if(mDataDir == "")
    SetDataDir(".");

  if(use_aliroot == false){

    string vsd_file (GForm("%s/%s", mDataDir.Data(), mVSDName.Data()));

    if(gSystem->AccessPathName(vsd_file.c_str(), kReadPermission) == false) {
      // Read OK!
      mFile      = new TFile(vsd_file.c_str());
      mDirectory = (TDirectory*)mFile->Get(GForm("Event%d", mEvent));
    } else {
      use_aliroot = true; 
      printf("VSD data does not exist, falling back to AliRunLoader.\n");
    }
  }

 
  if(use_aliroot && pRunLoader == 0){
    printf("Start importing AliRunLoader from dir %s\n", mDataDir.Data());

    string galice_file (GForm("%s/galice.root", mDataDir.Data()));

    GTime time(GTime::I_Now);
    if(gSystem->AccessPathName(galice_file.c_str(), kReadPermission)) {
      // CAN'T Read
      throw(_eh + "Can not access file '" + galice_file + "'.");
    }
    pRunLoader = AliRunLoader::Open(galice_file.c_str());
    pRunLoader->LoadgAlice();
    pRunLoader->LoadHeader();
    pRunLoader->LoadHits();
    pRunLoader->LoadKinematics();
    pRunLoader->LoadTrackRefs();
    GTime delta = time.TimeUntilNow();
    printf("AliRunLoader::Open took %lf sec\n", delta.ToDouble());

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

  } else {

    mTreeK = (TTree*)mDirectory->Get("Kinematics");
    //printf("treeK %p \n", mTreeK);
    if(mTreeK == 0) {
      printf("ERROR importing treek \n");
      return;
    }
    mTreeK->SetBranchAddress("P", &mpP);

    mTreeH =  (TTree*)mDirectory->Get("AliDetHit");
    //printf("TreeH %p \n", mTreeH);
    if (mTreeH == 0){
      printf("ERROR importing hits\n");
      return;
    }
    mTreeH->SetBranchAddress("H", &mpH);

  }
}

/**************************************************************************/

void ZAliLoad::Write()
{
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

  vsd->Write();
  vsd->Close();
  delete vsd;

  GledNS::PopFD();
}

/**************************************************************************/

void ZAliLoad::ResetEvent()
{
  RemoveLensesViaQueen(true);

  delete mTreeK;      mTreeK      = 0;
  delete mTreeH;      mTreeH      = 0;
  delete mTreeTR;     mTreeTR     = 0;
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
      delete gAlice; gAlice = 0; //!!!! dont know what it is use for
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
  if(pRunLoader == 0) {
    printf("AliRunLoader not imported 'n");
    return;
  }
  mTreeK = new TTree("Kinematics", "Sorted TParticles as in Alistack");
  mDirectory->Add(mTreeK);
 
  TTree* treek = pRunLoader->TreeK();
  TParticle tp, *_tp = &tp;
  treek->SetBranchAddress("Particles", &_tp);
  TEventList evl("STDO");
  Int_t nprimary = treek->Draw(">>STDO","fMother[0]==-1");

  Int_t nentries = (Int_t)treek->GetEntries();
  vector<MCParticle>  vmc(nentries);
  for (Int_t idx=0; idx<nentries; idx++) {
    // index to entry
    Int_t ent = (idx < nprimary) ?
      idx + nentries - nprimary :
      idx - nprimary;

    treek->GetEntry(ent);
    vmc[idx] = tp;
  }

  // read track refrences 
  TTree* mTreeTR =  pRunLoader->TreeTR();

  TClonesArray* RunArrayTR = new TClonesArray("AliTrackReference");
  mTreeTR->GetBranch("AliRun")->SetAddress(&RunArrayTR);

  Int_t nPrimaries = (Int_t) mTreeTR->GetEntries();
  for (Int_t iPrimPart = 0; iPrimPart<nPrimaries; iPrimPart++) {
    mTreeTR->GetEntry(iPrimPart);
    for (Int_t iTrackRef = 0; iTrackRef < RunArrayTR->GetEntriesFast(); iTrackRef++) {
      AliTrackReference *trackRef = (AliTrackReference*)RunArrayTR->At(iTrackRef); 
      Int_t track = trackRef->GetTrack();
      MCParticle& mcp = vmc[track];
      if(track < treek->GetEntries()){

	if(trackRef->TestBit(kNotDeleted)) {
	  mcp.bDecayed=true;
	  mcp.fDt=trackRef->GetTime();
	  mcp.fDx=trackRef->X(); mcp.fDy=trackRef->Y(); mcp.fDz=trackRef->Z();
	  mcp.fDPx=trackRef->Px(); mcp.fDPy=trackRef->Py(); mcp.fDPz=trackRef->Pz();
	  // if(mcp.GetPDG()->Stable()) printf("WARNING import TR %s track %d stable , but decayed \n", mcp.GetName(), track);
	  if(mcp.GetPdgCode() == 11)  mcp.bDecayed=false; // a bug in TreeTR
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

void ZAliLoad::PrintTreeK()
{
  MCParticle*  m_P = &mP;
  Int_t nentries = (Int_t)mTreeK->GetEntries();
  printf("<< print treek  %d entries in branch  \n",nentries);

  for (Int_t i=0;i<nentries;i++) {
    mTreeK->GetEntry(i);
    printf("<< print treek  %s p(%f) PGD(%d) Mother %d \n", m_P->GetName(), m_P->P(), m_P->GetPdgCode(), m_P->GetMother(0));
  }
}

/**************************************************************************/

void ZAliLoad::SelectParticles(ZNode* holder, const Text_t* selection,
			       Bool_t import_daughters)
{
  static const string _eh("ZAliLoad::ImportParticlesWithSelection ");
  if(mTreeK == 0) 
    throw (_eh + "kinematics not available.");
  
  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mParticleSelection.Data();

  TEventList evl("STD");
  Int_t n = mTreeK->Draw(">>STD", selection);
  // printf("%d entries in selection %s \n", n,  selection);

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
      ZParticle* zp = new ZParticle(p, label, GForm("%d %s", label, p->GetName()));
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

  if(pRunLoader == 0)
    throw(_eh + "AliRunLoader not available.");

  if(mTreeH != 0)
    throw(_eh + "hits already loaded.");

  mTreeH =  new TTree("AliDetHit", "Hit objects read from detects array");
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
	    mH.det_ID=det.detidx;
	    mH.eva_ID=eva_idx;
	    mH.particle_ID=ah->Track();
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
	  mH.eva_ID=eva_idx;
	  mH.det_ID=det.detidx;
	  mH.particle_ID=ali_hit->GetTrack();
	  mH.x=ali_hit->X(); mH.y=ali_hit->Y(); mH.z=ali_hit->Z();
	  // mH.Dump();
	  mTreeH->Fill(); 
	}
      }
      break;
    } // end default 
    } // end switch
  } // end while

  // mpH = &mH;//!!!
}

/**************************************************************************/

void ZAliLoad::SelectHits(HitContainer* holder, const char* selection)
{
  static const string _eh("ZAliLoad::ImportHitsWithSelection ");

  if(mTreeH == 0) 
    throw (_eh + "hits not available.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mHitSelection.Data();

  TEventList evl("SH"); 
  Int_t n = mTreeH->Draw(">>SH", selection);
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
// TPC digits
/**************************************************************************/

TPCSegment* ZAliLoad::ShowTPCSegment(Int_t segment_id, ZNode* holder)
{
  if (mTPCDigInfo == 0) {
    AliTPCParam *par;
    TTree* tree;
    if(pRunLoader) {
      pRunLoader->CdGAFile();
      par=(AliTPCParam *)gDirectory->Get("75x40_100x60_150x60");
      pRunLoader->LoadDigits("TPC");
      tree = pRunLoader->GetTreeD("TPC", false);
    }
    else {
      TFile* f = new TFile(GForm("%s/aligled.root", mDataDir.Data()));
      par=(AliTPCParam *)f->Get("TPCParameter");
      f->Close();

      //gDirectory->cd();
      TFile* f2 = new TFile(GForm("%s/TPC.Digits.root", mDataDir.Data()));
      //gDirectory->cd();
      TDirectory* d = (TDirectory*)f2->Get(GForm("Event%d",mEvent));
      printf("file %p dir %p\n",f2,d);
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
    tpc->Move3(0,0, mTPCDigInfo->mParameter->GetZLength());
  } else {
    tpc->Move3(0,0, - mTPCDigInfo->mParameter->GetZLength());
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
