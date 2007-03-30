// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AliConverter
//
//

#include "AliConverter.h"
#include <TFile.h>
#include "AliConverter.c7"

#include <AliStack.h>
#include <AliTPCTrackHitsV2.h>
#include <AliTPCTrackHitsV2.h>
#include <AliPDG.h>
#include <AliHit.h>
#include <AliSimDigits.h>
#include <AliKalmanTrack.h>
#include <AliESD.h>
#include <AliESDv0.h>
#include <AliTPCclusterMI.h>
#include <AliTPCClustersRow.h>
#include <AliITS.h>
#include <AliITSclusterV2.h>
#include <AliTrackReference.h>
#include <AliESDkink.h>
#include <AliITSLoader.h>

#include <TMath.h>

map<Int_t, GenInfo*> gimap;

ClassImp(AliConverter);

/**************************************************************************/

void AliConverter::_init()
{
  // *** Set all links to 0 ***
  mKineType = KT_Standard;
  mDataDir  = ".";
  
  pRunLoader = 0;

  mTreeK  = 0;
  mTreeH  = 0;
  mTreeTR = 0;
  mTreeC  = 0;
  mTreeR  = 0;
  mTreeV0 = 0;
  mTreeKK = 0;
  mTreeGI = 0;
  mpP  = &mP;
  mpH  = &mH;
  mpC  = &mC;
  mpV0 = &mV0;
  mpKK = &mKK;
  mpR  = &mR;
  mpGI = &mGI;

  mTPCHitRes = 2;
  mTRDHitRes = 2;

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
  // AliKalmanTrack::SetConvConst(1); 
}

/**************************************************************************/

void AliConverter::CreateVSD(const Text_t* data_dir, Int_t event,
			     const Text_t* vsd_file)
{
  static const Exc_t _eh("AliConverter::CreateVSD ");

  mDataDir = data_dir;
  mEvent   = event;

  string galice_file (GForm("%s/galice.root", mDataDir.Data()));
 
  // printf("Acces file to open runloader %s \n", mDataDir.Data());

  if(gSystem->AccessPathName(galice_file.c_str(), kReadPermission)) {
    throw(_eh + "Can not read file '" + galice_file + "'.");
  }
  pRunLoader = AliRunLoader::Open(galice_file.c_str());
  if(pRunLoader == 0)
    throw(_eh + "AliRunLoader::Open failed.");

  pRunLoader->LoadgAlice();
  Int_t status = pRunLoader->GetEvent(mEvent);
  if(status)
    throw(_eh + GForm("GetEvent(%d) failed, exit code %s.", mEvent, status));

  pRunLoader->LoadHeader();
  pRunLoader->LoadKinematics();
  pRunLoader->LoadTrackRefs();
  pRunLoader->LoadHits();

  GledNS::PushFD();

  mFile = TFile::Open(vsd_file, "RECREATE", "ALICE VisualizationDataSummary");
  mDirectory = new TDirectory("Event0", "");
  mDirectory->cd();

  try {
    // ConvertKinematics();
    ConvertKinematicsFromStack();
  } catch(Exc_t& exc) { warn_caller(exc); }

  try {
    ConvertHits();
  } catch(Exc_t& exc) { warn_caller(exc); }

  try {
    ConvertClusters();
  } catch(Exc_t& exc) { warn_caller(exc); }

  // Bugger, incompatible AliROOT for the new (12.2006) central event.
  // printf("AliConverter::CreateVSD GREPME ... aborting forcefully with bug-squash intentions.\n");
  // goto end_esd_processing;

  try {
    ConvertRecTracks();
  } catch(Exc_t& exc) {
    warn_caller(exc + " Skipping V0 extraction.");
    goto end_esd_processing;
  }

  try {
    ConvertV0();
  } catch(Exc_t& exc) { warn_caller(exc); }

  try {
    ConvertKinks();
  } catch(Exc_t& exc) { warn_caller(exc); }

 end_esd_processing:

  try {
    ConvertGenInfo();
  } catch(Exc_t& exc) { warn_caller(exc); }
 
  mFile->Write();  
  mFile->Close();
  delete mFile; 
  mFile=0, mDirectory =0;

  GledNS::PopFD();

  // clean after the VSD data was sucessfuly 
  // written
  mTreeK      = 0;
  mTreeH      = 0;
  mTreeTR     = 0;
  mTreeC      = 0;
  mTreeV0     = 0;
  mTreeKK     = 0;
  mTreeR      = 0;
  mTreeGI     = 0;

  pRunLoader->UnloadAll();

  // Crashing on 19.10.2006 (in deletion of tpcparamsrcrap)
  // delete pRunLoader;
  //if(gAlice) {
  //  delete gAlice; gAlice = 0;
  //}
  gAlice = 0;

  pRunLoader = 0;
}

/**************************************************************************/
// Kinematics
/**************************************************************************/

void AliConverter::ConvertKinematics()
{
  static const Exc_t _eh("AliConverter::ConvertKinematics ");

  if(mTreeK != 0) 
    throw (_eh + "kinematics already converted");

  TTree* treek = pRunLoader->TreeK();
  if(treek == 0) {
    warn_caller(_eh + "no kinematics.");
    return;
  }
  TParticle tp, *_tp = &tp;
  treek->SetBranchAddress("Particles", &_tp);

  mDirectory->cd();
  mTreeK = new TTree("Kinematics", "TParticles sorted by Label");
 
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
    vmc[idx] = tp;
    vmc[idx].SetLabel(idx);
  }

  // read track refrences 
  TTree* mTreeTR =  pRunLoader->TreeTR();

  if(mTreeTR == 0) {
    warn_caller(_eh + "no TrackRefs; some data will not be available.");
  } else {
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
	    if(TMath::Abs(mcp.GetPdgCode()) == 11)  mcp.SetDecayed(false); // a bug in TreeTR
	  }
	}       
      }
    }
  }

  mTreeK->Branch("P", "MCParticle",  &mpP, 512*1024, 99);

  for(vector<MCParticle>::iterator k=vmc.begin(); k!=vmc.end(); ++k) {
    MCParticle& mcp = *k;
    mP = mcp;

    TParticle* m  = &mcp;
    Int_t      mi = mcp.GetLabel();
    while(m->GetMother(0) != -1) {
      mi = m->GetMother(0);
      m = &vmc[mi];
    }
    mP.SetEvaLabel(mi);

    mTreeK->Fill();
  }

  mTreeK->BuildIndex("fLabel");
}

void AliConverter::ConvertKinematicsFromStack()
{
  static const Exc_t _eh("AliConverter::ConvertKinematicsFromStack ");

  if(mTreeK != 0) 
    throw (_eh + "kinematics already converted");

  AliStack* stack = pRunLoader->Stack();
  if(stack == 0)
    throw(_eh + "stack is null.");

  mDirectory->cd();
  mTreeK = new TTree("Kinematics", "TParticles sorted by Label");
 
  // Int_t nprimary = stack->GetNprimary();
  Int_t nentries = stack->GetNtrack();

  vector<MCParticle>  vmc(nentries);
  for (Int_t idx=0; idx<nentries; idx++) {
    TParticle* tp = stack->Particle(idx);
    vmc[idx] = *tp;
    vmc[idx].SetLabel(idx);
  }

  // read track refrences 
  TTree* mTreeTR =  pRunLoader->TreeTR();

  if(mTreeTR == 0) {
    warn_caller(_eh + "no TrackRefs; some data will not be available.");
  } else {
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
	    if(TMath::Abs(mcp.GetPdgCode()) == 11)  mcp.SetDecayed(false); // a bug in TreeTR
	  }
	}       
      }
    }
  }

  mTreeK->Branch("P", "MCParticle",  &mpP, 512*1024, 99);

  for(vector<MCParticle>::iterator k=vmc.begin(); k!=vmc.end(); ++k) {
    MCParticle& mcp = *k;
    mP = mcp;

    TParticle* m  = &mcp;
    Int_t      mi = mcp.GetLabel();
    while(m->GetMother(0) != -1) {
      mi = m->GetMother(0);
      m = &vmc[mi];
    }
    mP.SetEvaLabel(mi);

    mTreeK->Fill();
  }

  mTreeK->BuildIndex("fLabel");
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

/**************************************************************************/

void AliConverter::ConvertHits()
{
  static const Exc_t _eh("AliConverter::ConvertHits ");

  if(mTreeH != 0)
    throw(_eh + "hits already converted.");

  mDirectory->cd();
  mTreeH =  new TTree("Hits", "Combined detector hits.");
  Hit::Class()->IgnoreTObjectStreamer(true);
  mTreeH->Branch("H", "Hit", &mpH, 512*1024, 1);
 
  map<Int_t, Int_t> hmap;
  // parameters for ITS, TPC hits filtering
  Float_t x,y,z, x1,y1,z1;
  Float_t tpc_sqr_res = mTPCHitRes*mTPCHitRes;
  Float_t trd_sqr_res = mTRDHitRes*mTRDHitRes;

  int l=0;
  // load hits from the rest of detectors
  while(detects[l].name != 0) {
    Detector& det = detects[l++];

    switch(det.detidx) {
    case 1: { 
      Int_t count = 0;
      TTree* treeh = pRunLoader->GetTreeH(det.name, false);
      if(treeh == 0) {
	warn_caller(_eh + "no hits for "+ det.name +".");
	continue;
      }
      AliTPCTrackHitsV2 hv2, *_hv2=&hv2; 
      treeh->SetBranchAddress("TPC2", &_hv2);
      Int_t np = treeh->GetEntries();
      for(Int_t i=0; i<np; i++){
	treeh->GetEntry(i);
	Int_t eva_idx = np -i -1;
	if (hv2.First() == 0) continue;
        x = y = z = 0;
	do {
	  AliHit* ah = hv2.GetHit();
	  x1=ah->X();y1=ah->Y();z1=ah->Z();
	  if((x-x1)*(x-x1)+(y-y1)*(y-y1)+(z-z1)*(z-z1) > tpc_sqr_res) {
	    mH.fDetID    = det.detidx;
	    mH.fEvaLabel = eva_idx;
	    mH.fLabel    = ah->Track();
	    mH.x = x1; mH.y = y1; mH.z = z1;
	    mTreeH->Fill();
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
      if(treeh == 0) {
	warn_caller(_eh + "no hits for "+ det.name +".");
	continue;
      }
      TClonesArray *arr = new TClonesArray(det.hitbranch);
      treeh->SetBranchAddress(det.name, &arr);
      Int_t np = treeh->GetEntries();
      // in TreeH files hits are grouped in clones arrays
      // each eva particle has its own clone array 
      for (Int_t i=0; i<np; i++) {
	treeh->GetEntry(i);
	Int_t eva_idx = np -i -1;
	Int_t nh=arr->GetEntriesFast();
	x = y = z = 0;
	// printf("%d entry %d hits for primary %d \n", i, nh, eva_idx);
	for (Int_t j=0; j<nh; j++) {
	  AliHit* ali_hit = (AliHit*)arr->UncheckedAt(j);
	  mH.fEvaLabel = eva_idx;
	  mH.fDetID    = det.detidx;
	  mH.fLabel    = ali_hit->GetTrack();
	  mH.x=ali_hit->X(); mH.y=ali_hit->Y(); mH.z=ali_hit->Z();
          if(det.detidx == 2) {
	    x1=ali_hit->X();y1=ali_hit->Y();z1=ali_hit->Z();
	    if((x-x1)*(x-x1)+(y-y1)*(y-y1)+(z-z1)*(z-z1) < trd_sqr_res) continue;
	    x=x1; y=y1; z=z1;
	  } 
	  hmap[mH.fLabel]++;
	  mTreeH->Fill(); 
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
// Clusters
/**************************************************************************/

void AliConverter::ConvertClusters()
{
  static const Exc_t _eh("AliConverter::ConvertClusters ");

  if(mTreeC != 0)
    throw(_eh + "clusters already converted.");

  mDirectory->cd();
  mTreeC =  new TTree("Clusters", "rec clusters");
  mTreeC->Branch("C", "Hit", &mpC, 128*1024, 1);

  try {
    ConvertITSClusters();
  } catch(Exc_t& exc) { warn_caller(exc); }

  try {
    ConvertTPCClusters();
  } catch(Exc_t& exc) { warn_caller(exc); }
}

/**************************************************************************/

void AliConverter::ConvertTPCClusters()
{
  static const Exc_t _eh("AliConverter::ConvertTPCClusters ");

  auto_ptr<TFile> f 
    ( TFile::Open(GForm("%s/TPC.RecPoints.root", mDataDir.Data())) );
  if(!f.get())
    throw(_eh + "can not open 'TPC.RecPoints.root' file.");
    
  auto_ptr<TDirectory> d
    ( (TDirectory*) f->Get(GForm("Event%d", mEvent)) );
  if(!d.get())
    throw(_eh + GForm("event directory '%d' not found.", 0));

  auto_ptr<TTree> tree( (TTree*) d->Get("TreeR") );
  if(!tree.get())
    throw(_eh + "'TreeR' not found.");

  // Crashing on 19.10.2006 (in deletion of tpcparamsrcrap)
  // auto_ptr<AliTPCParam> par( get_tpc_param(_eh) );
  AliTPCParam* par ( get_tpc_param(_eh) );

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

void AliConverter::ConvertITSClusters()
{
  static const Exc_t _eh("AliConverter::ConvertITSClusters ");

  auto_ptr<TFile> f 
    ( TFile::Open(GForm("%s/ITS.RecPoints.root", mDataDir.Data())) );
  if(!f.get())
    throw(_eh + "can not open 'ITS.RecPoints.root' file.");
    
  auto_ptr<TDirectory> d
    ( (TDirectory*) f->Get(GForm("Event%d", mEvent)) );
  if(!d.get())
    throw(_eh + GForm("event directory '%d' not found.", 0));

  auto_ptr<TTree> tree( (TTree*) d->Get("TreeR") );
  if(!tree.get())
    throw(_eh + "'TreeR' not found.");

  AliITSLoader* ITSld =  (AliITSLoader*) pRunLoader->GetLoader("ITSLoader");
  //AliITS* pITS = ITSld->GetITS();
  AliITSgeom* geom = ITSld->GetITSgeom();
  //AliITSgeom* geom = new AliITSgeom();
  //geom->ReadNewFile("/home/aljam/ITSgeometry.det");

  //printf("alice ITS geom %p \n",geom );

  if(!geom)
    throw(_eh + "can not find ITS geometry");

  TClonesArray *arr = new TClonesArray("AliITSclusterV2");
  tree->SetBranchAddress("Clusters", &arr);
  Int_t nmods = tree->GetEntries();
  Float_t gc[3];
  map<Int_t, Int_t> cmap;

  for (Int_t mod=0; mod<nmods; mod++) {
    tree->GetEntry(mod);
    Int_t nc=arr->GetEntriesFast();
    for (Int_t j=0; j<nc; j++) {
      AliITSclusterV2* recp = (AliITSclusterV2*)arr->UncheckedAt(j);

      Double_t rot[9];     
      geom->GetRotMatrix(mod,rot);
      Int_t lay,lad,det;   
      geom->GetModuleId(mod,lay,lad,det);
      Float_t tx,ty,tz;    
      geom->GetTrans(lay,lad,det,tx,ty,tz);     

      Double_t alpha=TMath::ATan2(rot[1],rot[0])+TMath::Pi();
      Double_t phi1=TMath::Pi()/2+alpha;
      if(lay==1) phi1+=TMath::Pi();

      Float_t cp=TMath::Cos(phi1), sp=TMath::Sin(phi1);
      Float_t  r=tx*cp+ty*sp;
      gc[0]= r*cp - recp->GetY()*sp;
      gc[1]= r*sp + recp->GetY()*cp;
      gc[2]= recp->GetZ();
      // write in mTreeC  tree
      mC.fDetID = 0;
      mC.fLabel = recp->GetLabel(0);
      mC.x      = r*cp - recp->GetY()*sp;
      mC.y      = r*sp + recp->GetY()*cp;
      mC.z      = recp->GetZ();
      mTreeC->Fill();
      cmap[mC.fLabel]++;
    } 

    for(map<Int_t, Int_t>::iterator j=cmap.begin(); j!=cmap.end(); ++j) {
      get_geninfo(j->first)->Nc += j->second;
    }
  }
  delete arr;
}

/**************************************************************************/
// ESD
/**************************************************************************/

void AliConverter::ConvertRecTracks()
{
  static const Exc_t _eh("AliConverter::ConvertRecTracks ");

  if(mTreeR != 0)
    throw(_eh + "tracks already converted.");

  mDirectory->cd();
  mTreeR =  new TTree("RecTracks", "rec tracks");

  ESDParticle::Class()->IgnoreTObjectStreamer(true);
  mTreeR->Branch("R", "ESDParticle", &mpR, 512*1024,1);
 
  TFile f(GForm("%s/AliESDs.root", mDataDir.Data()));
  if(!f.IsOpen())
    throw(_eh + "no AliESDs.root file.");

  TTree* tree = (TTree*) f.Get("esdTree");
  if (tree == 0) 
    throw(_eh + "no esdTree.");

 
  AliESD *fEvent=0;  
  tree->SetBranchAddress("ESD", &fEvent);
  tree->GetEntry(mEvent); 

 
  // reconstructed tracks
  AliESDtrack* esd_t;
  for (Int_t n =0; n< fEvent->GetNumberOfTracks();n++) {
    esd_t = fEvent->GetTrack(n);
    esd_t->GetXYZ(mpR->fV);
    esd_t->GetPxPyPz(mpR->fP);
    mpR->fSign  = (Int_t) esd_t->GetSign();
    mpR->fLabel = esd_t->GetLabel();
    mTreeR->Fill();
  }
  mTreeR->BuildIndex("fLabel");
}

/**************************************************************************/

void AliConverter::ConvertV0()
{
  static const Exc_t _eh("AliConverter::ConvertV0 ");

  if(mTreeV0 != 0)
    throw(_eh + "V0 already converted.");

  mDirectory->cd();
  mTreeV0 =  new TTree("V0", "V0 points");

  ESDParticle::Class()->IgnoreTObjectStreamer(true);
  mTreeV0->Branch("V0", "V0", &mpV0, 512*1024,1);

  TFile f(GForm("%s/AliESDs.root", mDataDir.Data()));
  if(!f.IsOpen()){
    throw(_eh + "no AliESDs.root file.");
  }

  TTree* tree = (TTree*) f.Get("esdTree");
  if (tree == 0) 
    throw(_eh + "no esdTree.");

  AliESD *fEvent=0;  
  tree->SetBranchAddress("ESD", &fEvent);
  tree->GetEntry(mEvent); 

  for (Int_t n =0; n< fEvent->GetNumberOfV0s(); n++) {
    AliESDv0* av = fEvent->GetV0(n);
    AliESDtrack *trackN = fEvent->GetTrack(av->GetNindex()); // negative daughter
    AliESDtrack *trackP = fEvent->GetTrack(av->GetPindex()); // positive daughter

    Double_t x,y,cos,sin; 

    Double_t A[3];

    mV0.fStatus = av->GetStatus();
    // Point of closest approach
    av->GetXYZ(A[0], A[1], A[2]);
    mV0.fDCA[0] = A[0];
    mV0.fDCA[1] = A[1];
    mV0.fDCA[2] = A[2];
    // set birth vertex of neutral particle     
    av->GetXYZ(mV0.fV0[0], mV0.fV0[1], mV0.fV0[2]);

    // momentum and position of negative particle
    av->GetParamN()->GetPxPyPz(A);
    mV0.fPM[0] = A[0]; mV0.fPM[1] = A[1]; mV0.fPM[2] = A[2];
    // read AliExternalTrackParam
    x = av->GetParamN()->GetX(); 
    y = av->GetParamN()->GetY(); 
    cos = TMath::Cos( av->GetParamN()->GetAlpha());
    sin = TMath::Sin( av->GetParamN()->GetAlpha());
    mV0.fVM[0] = x*cos - y*sin;
    mV0.fVM[1] = x*sin + y*cos;
    mV0.fVM[2] = av->GetParamN()->GetZ();

    // momentum and position of positive particle
    av->GetParamP()->GetPxPyPz(A);
    mV0.fPP[0] = A[0]; mV0.fPP[1] = A[1]; mV0.fPP[2] = A[2];
    x = av->GetParamP()->GetX();
    y = av->GetParamP()->GetY();
    cos = TMath::Cos(av->GetParamP()->GetAlpha());
    sin = TMath::Sin(av->GetParamP()->GetAlpha());
    mV0.fVP[0] = x*cos - y*sin;
    mV0.fVP[1] = x*sin + y*cos;
    mV0.fVP[2] = av->GetParamP()->GetZ();

    // daughter indices
    mV0.fDLabels[0] = TMath::Abs(trackN->GetLabel());
    mV0.fDLabels[1] = TMath::Abs(trackP->GetLabel());

    mV0.fPDG = av->GetPdgCode();

    // printf("V0 convert labels(%d,%d) index(%d,%d)\n", 
    //	   av->GetLab(0), av->GetLab(1),
    //	   av->GetIndex(0), av->GetIndex(1));

    mTreeV0->Fill();
  }
  // if(fEvent->GetNumberOfV0MIs()) mTreeV0->BuildIndex("fLabel");
}

/**************************************************************************/

void AliConverter::ConvertKinks()
{
  static const Exc_t _eh("AliConverter::ConvertKinks ");

  if(mTreeKK != 0)
    throw(_eh + "Kinks already converted.");

  mDirectory->cd();
  mTreeKK =  new TTree("Kinks", "ESD Kinks");

  ESDParticle::Class()->IgnoreTObjectStreamer(true);
  mTreeKK->Branch("KK", "Kink", &mpKK, 512*1024,1);

  TFile f(GForm("%s/AliESDs.root", mDataDir.Data()));
  if(!f.IsOpen()){
    throw(_eh + "no AliESDs.root file.");
  }

  TTree* tree = (TTree*) f.Get("esdTree");
  if (tree == 0) 
    throw(_eh + "no esdTree.");

  AliESD *fEvent=0;  
  tree->SetBranchAddress("ESD", &fEvent);
  tree->GetEntry(mEvent); 

  //  printf("CONVERT KINK Read %d entries in tree kinks \n",  fEvent->GetNumberOfKinks());
  for (Int_t n =0; n< fEvent->GetNumberOfKinks(); n++) {
    AliESDkink* kk = fEvent->GetKink(n);
    Double_t x,y,cos,sin; 

    mKK.fLabel  = kk->GetLabel(0);
    mKK.fStatus = Int_t(kk->GetStatus(1) << 8 + kk->GetStatus(2));

    { // reconstructed kink position
      const Double_t* pos = kk->GetPosition();
      mKK.fKV[0] = pos[0]; mKK.fKV[1] = pos[1]; mKK.fKV[2] = pos[2];
    }

    { // momentum and position of mother 
      const AliExternalTrackParam& tp_mother = kk->RefParamMother();
      tp_mother.GetPxPyPz(mKK.fP);
      const Double_t* par =  tp_mother.GetParameter();
      // printf("KINK Pt %f, %f \n",1/kk->fParamMother.Pt(),par[4] );
      mKK.fSign = (par[4] < 0) ? -1 : 1;

      x = tp_mother.GetX(); 
      y = tp_mother.GetY(); 
      cos = TMath::Cos(tp_mother.GetAlpha());
      sin = TMath::Sin(tp_mother.GetAlpha());
      mKK.fV[0] = x*cos - y*sin;
      mKK.fV[1] = x*sin + y*cos;
      mKK.fV[2] = tp_mother.GetZ();
    }

    { // momentum and position of daughter 
      const AliExternalTrackParam& tp_daughter = kk->RefParamDaughter();
      tp_daughter.GetPxPyPz(mKK.fDP);

      x = tp_daughter.GetX(); 
      y = tp_daughter.GetY(); 
      cos = TMath::Cos(tp_daughter.GetAlpha());
      sin = TMath::Sin(tp_daughter.GetAlpha());
      mKK.fEV[0] = x*cos - y*sin;
      mKK.fEV[1] = x*sin + y*cos;
      mKK.fEV[2] = tp_daughter.GetZ();
    }

    mTreeKK->Fill();
  }
  if(fEvent->GetNumberOfKinks()) mTreeKK->BuildIndex("fLabel");
}
/**************************************************************************/
// GenInfo
/**************************************************************************/

void AliConverter::ConvertGenInfo()
{
  static const Exc_t _eh("AliConverter::ConvertGenInfo ");

  if(mTreeGI != 0)
    throw(_eh + "GI already converted.");

  mDirectory->cd();
  mTreeGI = new TTree("GenInfo", "Objects prepared for cross querry");

  GenInfo::Class()->IgnoreTObjectStreamer(true);
  mTreeGI->Branch("GI", "GenInfo", &mpGI, 512*1024, 99);
  mTreeGI->Branch("K.", &mpP);
  mTreeGI->Branch("R.", "ESDParticle",   &mpR);

  for(map<Int_t, GenInfo*>::iterator j=gimap.begin(); j!=gimap.end(); ++j) {
    mGI = *(j->second);
    mGI.fLabel = j->first;
    mTreeK->GetEntry(j->first);

    if(mTreeR) {
      Int_t re = mTreeR->GetEntryWithIndex(j->first); // was GetEntryNumberWithIndex
      if(re != -1) 
	mGI.bR = 1;
    }
    //    Int_t has_v0 =  mTreeV0->GetEntryNumberWithIndex(j->first);
    //if (has_v0 != -1)
    //  mGI.bV0 = 1;
    if (mTreeKK){
      Int_t has_kk =  mTreeKK->GetEntryWithIndex(j->first); // was GetEntryNumberWithIndex
      if (has_kk != -1)
	mGI.bKK = 1;
    }
    mTreeGI->Fill();
  }
  gimap.clear();
}

/**************************************************************************/
/**************************************************************************/
// Protected methods
/**************************************************************************/
/**************************************************************************/

AliTPCParam* AliConverter::get_tpc_param(const Exc_t& eh)
{
  auto_ptr<TFile> fp( TFile::Open(GForm("%s/galice.root", mDataDir.Data())) );
  if(!fp.get())
    throw(eh + "can not open 'galice.root' file.");
  AliTPCParam* par = (AliTPCParam *) fp->Get("75x40_100x60_150x60");
  if(!par)
    throw(eh + "TPC data not found.");
  return par;
}



GenInfo* AliConverter::get_geninfo(Int_t label)
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
