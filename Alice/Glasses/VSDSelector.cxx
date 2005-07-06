// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// VSDSelector
//
//

#include "VSDSelector.h"
#include "VSDSelector.c7"

#include <Glasses/ZGlass.h>
#include <Glasses/ZQueen.h>
#include <Glasses/RecTrack.h>
#include <Glasses/V0Track.h>
#include <Glasses/KinkTrack.h>
#include <Stones/TTreeTools.h>

ClassImp(VSDSelector)

/**************************************************************************/
void VSDSelector::_init()
{
  // *** Set all links to 0 ***
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
  mpR  = &mR;
  mpV0 = &mV0;
  mpKK = &mKK;
  mpGI = &mGI;
  mImportMode = 0;

  mParticleSelection = "fMother[0] == -1";
  mHitSelection      = "fDetID < 5";
  mClusterSelection  = "fDetID == 1";
  mRecSelection      = "Pt() > 0.1";
  mV0Selection       = "fStatus == 100";
  mKinkSelection     = "fDP[0] > 0";
  mGISelection       = "bR == 1";

  mFile    = 0; 
  mDirectory = 0;
}

/**************************************************************************/

void VSDSelector::LoadVSD(const Text_t* vsd_file_name)
{
  static const string _eh("VSDSelector::LoadVSD ");
  
  mFile = TFile::Open(vsd_file_name);
  if(!mFile)
    throw(_eh + GForm("can not open VSD file '%s'.", vsd_file_name));


  string event_dir("Event0"); // (GForm("Event%d", mEvent));
  mDirectory = (TDirectory*) mFile->Get(event_dir.c_str());


  printf("Reading kinematics.\n");
  mTreeK = (TTree*) mDirectory->Get("Kinematics");
  if (mTreeK == 0) {
    printf("%s Kinematics not available in mDirectoryectory %s.\n", 
	   _eh.c_str(), mDirectory->GetName());
  } else {
    mTreeK->SetBranchAddress("P", &mpP);
  }
  printf("Reading hits.\n");  
  mTreeH = (TTree*) mDirectory->Get("Hits");
  if (mTreeH == 0) {
    printf("%s Hits not available in mDirectoryectory %s.\n", 
	   _eh.c_str(), mDirectory->GetName());
  } else {
    mTreeH->SetBranchAddress("H", &mpH);
  }

  printf("Reading clusters.\n");
  mTreeC = (TTree*) mDirectory->Get("Clusters");
  if (mTreeC == 0) {
    printf("%s Clusters not available in mDirectoryectory %s.\n", 
	   _eh.c_str(), mDirectory->GetName());
  } else {
    mTreeC->SetBranchAddress("C", &mpC);
  }

  printf("Reading reconstructed tracks.\n");
  mTreeR = (TTree*) mDirectory->Get("RecTracks");
  if (mTreeR == 0){ 
    printf("%s RecTracks not available in mDirectoryectory %s.\n", 
	   _eh.c_str(), mDirectory->GetName());
  } else {
    mTreeR->SetBranchAddress("R", &mpR);
  }

  printf("Reading V0 points. \n");
  mTreeV0 =  (TTree*) mDirectory->Get("V0");
  if (mTreeV0 == 0){
    printf("%s V0 not available in mDirectoryectory %s.\n", 
	   _eh.c_str(), mDirectory->GetName());
  } else {
    mTreeV0->SetBranchAddress("V0", &mpV0);
  }
 

  printf("Reading Kinks. \n");
  mTreeKK =  (TTree*) mDirectory->Get("Kinks");
  if (mTreeKK == 0){
    printf("%s Kinks not available in mDirectoryectory %s.\n", 
	   _eh.c_str(), mDirectory->GetName());
  } else {
    mTreeKK->SetBranchAddress("KK", &mpKK);
  }
   
  printf("Reading GenInfo.\n");
  mTreeGI = (TTree*)mDirectory->Get("GenInfo");
  if (mTreeGI == 0) {
    printf("%s GenInfo not available in mDirectoryectory %s.\n", 
	   _eh.c_str(), mDirectory->GetName());
  } else {
    mTreeGI->SetBranchAddress("GI", &mpGI);
    mTreeGI->SetBranchAddress("K.", &mpP);
    mTreeGI->SetBranchAddress("R.", &mpR);
  }
}

/**************************************************************************/
void VSDSelector::ResetEvent()
{
  RemoveLensesViaQueen(true);

  delete mTreeK;      mTreeK      = 0;
  delete mTreeH;      mTreeH      = 0;
  delete mTreeTR;     mTreeTR     = 0;
  delete mTreeC;      mTreeC      = 0;
  delete mTreeR;      mTreeR      = 0;
  delete mTreeV0;     mTreeV0     = 0;
  delete mTreeKK;     mTreeKK     = 0;
  delete mTreeGI;     mTreeGI     = 0;

  if (mFile) {
    mFile->Close();
    delete mFile;
    mFile = 0; mDirectory = 0;
  }
}

/**************************************************************************/

MCParticle* VSDSelector::Particle(Int_t i)
{
  static const string _eh("VSDSelector::Particle ");
  if(mTreeK == 0) 
    throw (_eh + "kinematics not available.");
  
  Int_t re = mTreeK->GetEntryNumberWithIndex(i);
  mTreeK->GetEntry(re);
  MCParticle* p = new MCParticle(mP); 
  return p;
}

/**************************************************************************/
//  selection methods
/**************************************************************************/
void VSDSelector::SelectParticles( ZNode* holder, const Text_t* selection, 
			       Bool_t import_daughters)
{
  static const string _eh("VSDSelector::SelectParticles ");

  if(mTreeK == 0) 
    throw (_eh + "kinematics not available.");
  
  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mParticleSelection.Data();

  TTreeQuery evl;
  Int_t n = evl.Select(mTreeK, selection);
  // printf("%d entries in selection '%s'.\n", n,  selection);

  if(n == 0)
    throw (_eh + "no entries found for selection in kinematics.");

  bool add_holder_p = false;
  if(holder == 0) {
    holder = new ZNode(GForm("Particles %s", selection));
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
void VSDSelector::SelectHits( ZNode* holder, const char* selection)
{
  static const string _eh("VSDSelector::SelectHits ");

  if(mTreeH == 0) 
    throw (_eh + "hits not available.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mHitSelection.Data();

  TTreeQuery evl;
  Int_t n = evl.Select(mTreeH, selection);
  // printf("ImportHitsWithSelection %d entries for selection %s\n", n, selection);
  
  if(n==0)
    throw(_eh + "no hits matching selection.");


  HitContainer* container = new HitContainer(GForm("%d Hits %s",n, selection));
  mQueen->CheckIn(container);

  container->Reset(n);
  for(Int_t i=0; i<n; i++) {
    const Int_t entry = evl.GetEntry(i);
    mTreeH->GetEntry(entry);
    container->SetPoint(i, entry, &mpH->x);
  }
  if(holder)
    holder->Add(container);
  else Add(container); 
}

/**************************************************************************/

void VSDSelector::SelectClusters(ZNode* holder, const char* selection)
{
  static const string _eh("VSDSelector::SelectClusters ");

  if(mTreeC == 0) 
    throw (_eh + "clusters not available.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mClusterSelection.Data();

  TTreeQuery evl;
  Int_t n = evl.Select(mTreeC, selection);

  if(n == 0)
    throw (_eh + "no entries found for selection in clusters.");

  HitContainer* container = new HitContainer(GForm("%d Clusters %s",n, selection));
  container->SetColor(1.,1.,0.,1.);
  mQueen->CheckIn(container);
  container->Reset(n);
  for(Int_t i=0; i<n; i++) {
    const Int_t entry = evl.GetEntry(i);
    mTreeC->GetEntry(entry);
    container->SetPoint(i, entry, &mpC->x);
  }
  if(holder)
    holder->Add(container);
  else Add(container);
}

/**************************************************************************/

void VSDSelector::SelectRecTracks(ZNode* holder, const Text_t* selection)
{
  static const string _eh("VSDSelector::SelectRecTracks ");

  if(mTreeR == 0) 
    throw (_eh + "reconstructed tracks not available.");
  
  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mRecSelection.Data();

  TTreeQuery evl;
  Int_t n = evl.Select(mTreeR, selection);
  // printf("%d entries in selection %s \n", n,  selection);

  if (n == 0)
    throw (_eh + "No entries found in ESD data.");

  bool add_holder_p = false;
  if(holder == 0) {
    holder = new ZNode(GForm("RecTracks %s", selection));
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

/**************************************************************************/

void VSDSelector::SelectV0(ZNode* holder, const Text_t* selection,
			   Bool_t import_kine)
{
  static const string _eh("VSDSelector::SelectV0 ");

  if(mTreeV0 == 0) 
    throw (_eh + "V0 tree not available.");
  
  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mV0Selection.Data();

  TTreeQuery evl;
  Int_t n = evl.Select(mTreeV0, selection);
  // printf("%d entries in selection %s \n", n,  selection);
 
  if(n==0)
    throw (_eh + "no entries found for selection.");

  bool add_holder_p = false;
  if(holder == 0) {
    holder = new ZNode(GForm("V0 %s", selection));
    mQueen->CheckIn(holder);
    add_holder_p = true;
    Add(holder);
  }

  if(n > 0) {
    for (Int_t i=0; i<n; i++){
      mTreeV0->GetEntry( evl.GetEntry(i));
      // neutral mother
      Double_t pp[3];
      pp[0]= mV0.fPP[0]+mV0.fPM[0];
      pp[1]= mV0.fPP[1]+mV0.fPM[1];
      pp[2]= mV0.fPP[2]+mV0.fPM[2];
      ESDParticle* v = new ESDParticle(&mV0.fV0[0], &pp[0],-1, 0);
      v->fV[0]=mV0.fDCA[0]; v->fV[1]=mV0.fDCA[1]; v->fV[2] = mV0.fDCA[2];
      V0Track* tV0 = new V0Track(v); 
      tV0->mVM[0]=mV0.fVM[0]; tV0->mVM[1]=mV0.fVM[1]; tV0->mVM[2]=mV0.fVM[2];
      tV0->mVP[0]=mV0.fVP[0]; tV0->mVP[1]=mV0.fVP[1]; tV0->mVP[2]=mV0.fVP[2];
      tV0->SetName(GForm("V0 %d : %d {%d}", mV0.fDLabels[0], mV0.fDLabels[1], mV0.fPDG));
      mQueen->CheckIn(tV0);
      holder->Add(tV0);
      // minus rec daughter
      ESDParticle* m = new ESDParticle(&mV0.fVM[0], &mV0.fPM[0], mV0.fDLabels[0], -1);
      RecTrack* tM = new RecTrack(m); 
      mQueen->CheckIn(tM);
      tV0->Add(tM);
      // plus rec daughter
      ESDParticle* p = new ESDParticle(&mV0.fVP[0], &mV0.fPP[0], mV0.fDLabels[1], 1);
      RecTrack* tP = new RecTrack(p); 
      mQueen->CheckIn(tP);
      tV0->Add(tP);

      //kinematics
      if(import_kine){
	holder->SetName(GForm("V0&Kine %s", selection));
	// minus kine daughter
	MCParticle* mk = Particle(mV0.fDLabels[0]);
	MCTrack* mc_mk = new MCTrack(mk);
	mQueen->CheckIn(mc_mk);
	tV0->Add(mc_mk);
	// plus kine daughter
	MCParticle* pk = Particle(mV0.fDLabels[1]);
	MCTrack* mc_pk = new MCTrack(pk);
	mQueen->CheckIn(mc_pk);
	tV0->Add(mc_pk);
	// check for kine mother
	if (mk->GetFirstMother() == pk->GetFirstMother()){
	  MCParticle* k = Particle(pk->GetFirstMother());
	  MCTrack* mc_k = new MCTrack(k);
	  mQueen->CheckIn(mc_k);
	  tV0->Add(mc_k);
	}
      }
    }
  }
}


/**************************************************************************/

void VSDSelector::SelectKinks(ZNode* holder, const Text_t* selection,
			      Bool_t import_kine)
{
  static const string _eh("VSDSelector::SelectKinks ");

  if(mTreeKK == 0) 
    throw (_eh + "Kinks tree not available.");
  
  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mKinkSelection.Data();

  TTreeQuery evl;
  Int_t n = evl.Select(mTreeKK, selection);
  // printf("%d entries in selection %s \n", n,  selection);
 
  if(n==0)
    throw (_eh + "no entries found for selection.");

  bool add_holder_p = false;
  if(holder == 0) {
    holder = new ZNode(GForm("Kinks %s", selection));
    mQueen->CheckIn(holder);
    add_holder_p = true;
    Add(holder);
  }

  if(n > 0) {
    Kink* k;
    KinkTrack* kt;
    for (Int_t i=0; i<n; i++){
      Int_t label = evl.GetEntry(i);
      mTreeKK->GetEntry(label);
      k = new Kink(*mpKK); 
      kt = new KinkTrack(k); 
      mQueen->CheckIn(kt);
      holder->Add(kt);
    
      //kinematics
      if(import_kine){
	printf("kinks & import kine");
	//  kine mother
	MCParticle* mk = Particle(k->fLabel);
	MCTrack* mc_mk = new MCTrack(mk);
	mQueen->CheckIn(mc_mk);
	kt->Add(mc_mk);
	//  kine daughter
	MCParticle* pk = Particle(k->fDLabel);
	MCTrack* mc_pk = new MCTrack(pk);
	mQueen->CheckIn(mc_pk);
	kt->Add(mc_pk);
      }
    }
  }
}

/**************************************************************************/

void VSDSelector::SelectGenInfo( ZNode* holder, const Text_t* selection)
{
  static const string _eh("VSDSelector::SelectGenInfo ");

  if(mTreeGI == 0)
    throw (_eh + "mTreeGI not available.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mGISelection.Data();

  TTreeQuery evl;
  Int_t nlabels = evl.Select(mTreeGI, selection);
  // printf("%d entries in selection %s \n", nlabels,  selection);

  if(nlabels == 0) {
    throw (_eh + "No entries match selection in  mTreeGI.");
  }

  if(holder == 0) {
    holder = new ZNode(GForm("GI %s", selection));
    mQueen->CheckIn(holder);
    Add(holder);
  }

  // create new import mode object, if not set alrady
  if(mImportMode == 0) {
    GIImportStyle* s = new GIImportStyle;
    mQueen->CheckIn(s);
    SetImportMode(s);
  }

  // holders for reconstructed and MC particles
  ZNode* mc_holder  =  new ZNode(GForm("MC  %s", selection));
  ZNode* rec_holder =  new ZNode(GForm("Rec %s", selection));
  mQueen->CheckIn(mc_holder); mQueen->CheckIn(rec_holder);
  if(mImportMode->mImportKine)
    holder->Add(mc_holder); 
  if(mImportMode->mImportRec)
    holder->Add(rec_holder);

  Int_t nc = 0, nh = 0;
  // Int_t labels[nlabels];
  set<Int_t> labels;

  for (Int_t i=0; i<nlabels; i++){
    mTreeGI->GetEntry(evl.GetEntry(i));
    labels.insert(mGI.fLabel);
    if( mImportMode->mImportKine){
      MCParticle* p = new MCParticle(mP); // check if gimap exists
      MCTrack* zp  = new MCTrack(p);
      mQueen->CheckIn(zp);
      mc_holder->Add(zp); 
      mc_holder->SetRnrElements(mImportMode->mRnrKine);
    }
    if(mGI.bR == 1 &&  mImportMode->mImportRec){
      // printf("Created rec track %d \n", mGI.fLabel);
      ESDParticle* t = new ESDParticle(mR);
      RecTrack* rt = new RecTrack(t);
      mQueen->CheckIn(rt);
      rec_holder->Add(rt);
      rec_holder->SetRnrElements(mImportMode->mRnrRec);
    }
     
    nh += mpGI->Nh;
    nc += mpGI->Nc;
  }

  if(nh > 0 && mImportMode->mImportHits) {
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
    hcont->SetRnrSelf( mImportMode->mRnrHits);
    mQueen->CheckIn(hcont);    
    holder->Add(hcont);
  }
  
  // printf("Selecting clusters in GI select \n");
  if(nc > 0 && mImportMode->mImportClusters) {
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
    ccont->SetRnrSelf(mImportMode->mRnrClusters);
    mQueen->CheckIn(ccont);
    holder->Add(ccont);
  }
}

