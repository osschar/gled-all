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

#include <Glasses/ITSModule.h>
#include <Glasses/ITSScaledModule.h>
#include <Glasses/TOFSector.h>
#include <Gled/GledTypes.h>

#include <AliTPCParam.h>

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
  mITSDigInfo = 0;
  mTPCDigInfo = 0;
  mTOFDigInfo = 0;

  mConverter= 0;
  mSelector = 0;

  mDataDir  = ".";
  mEvent    = 0;
  mKineType = AliConverter::KT_Standard;

  mDefVSDName = "AliVSD.root";
  mVSDFile    = "";

  mParticleSelection = "fMother[0] == -1";
  mHitSelection      = "fDetID < 5";
  mClusterSelection  = "fDetID == 1";
  mRecSelection      = "Pt() > 0.1";
  mV0Selection       = "fStatus == 100";
  mKinkSelection       = "fStatus > 0";
  mGISelection       = "bR == 1";

  mOperation = "<idle>";
}

/**************************************************************************/
// VSD handling
/**************************************************************************/

Bool_t ZAliLoad::check_read(const string& file)
{
  return (gSystem->AccessPathName(file.c_str(), kReadPermission) == false);
}

string ZAliLoad::get_vsd_name(Bool_t check_p)
{
  m_auto_vsdfile_p = false;

  string file(mVSDFile.Data());
  if(file != "") {
    if(!check_p || (check_p && check_read(file))) {
      return file;
    }
  }

  m_auto_vsdfile_p = true;

  string there(GForm("%s/%s", mDataDir.Data(), mDefVSDName.Data()));
  if(!check_p || (check_p && check_read(there)))
    return there;

  string here(mDefVSDName.Data());
  if(!check_p || (check_p && check_read(here)))
    return here;

  throw(file + " | " + there + " | " + here);
}

/**************************************************************************/

void ZAliLoad::CreateVSD()
{
  OpMutexHolder omh(this, "CreateVSD");

  string vsd_file = get_vsd_name(false);
  SetVSDFile(vsd_file.c_str());
  if(!mConverter) {
    AliConverter* c = new AliConverter;
    mQueen->CheckIn(c);
    SetConverter(c);
  }
  mConverter->SetKineType(mKineType);
  // printf("Write Converter in VSD file %s , data dir %s\n",mVSDFile.Data(),mDataDir.Data() );
  mConverter->CreateVSD(mDataDir, mEvent, mVSDFile);
}

void ZAliLoad::LoadVSD()
{
  static const string _eh("ZAliLoad::LoadVSD ");

  OpMutexHolder omh(this, "LoadVSD");

  string vsd_file;
  try {
    vsd_file = get_vsd_name(true);
    SetVSDFile(vsd_file.c_str());
  }
  catch(string exc) {
    CreateVSD();
  }
  mSelector = new VSDSelector(GForm("VSD %s", mDataDir.Data()));
  mQueen->CheckIn(mSelector); Add(mSelector);
  mSelector->LoadVSD(mVSDFile);
}

/**************************************************************************/

void ZAliLoad::ClearData()
{
  OpMutexHolder omh(this, "ResetData()");
 
  SetConverter(0);
  SetSelector(0);
  RemoveLensesViaQueen(true);
 
  if (mTPCDigInfo) {
    delete mTPCDigInfo; mTPCDigInfo = 0;
  }
  if (mITSDigInfo) {
    delete mITSDigInfo; mITSDigInfo = 0;
  }
  if (mTOFDigInfo) {
    delete mITSDigInfo; mITSDigInfo = 0;
  }
  SetDataDir(".");
  if(m_auto_vsdfile_p = true)
    SetVSDFile("");
}

/**************************************************************************/
// DIGITS
/**************************************************************************/

void ZAliLoad::check_tpcdig_info()
{
  if (mTPCDigInfo != 0 &&
      (mTPCDigInfo->GetDataDir() != mDataDir || 
       mTPCDigInfo->GetEvent()   != mEvent))
    { 
      mTPCDigInfo->DecRefCount();
      mTPCDigInfo = 0;
    }
  if (mTPCDigInfo == 0)
    {
      mTPCDigInfo = new TPCDigitsInfo();
      mTPCDigInfo->IncRefCount();
      mTPCDigInfo->SetData(mDataDir, mEvent);
    }
}

TPCSegment* ZAliLoad::ShowTPCSegment(Int_t segment_id, ZNode* holder)
{
  static const string _eh("ZAliLoad::ShowTPCSegment ");

  check_tpcdig_info();

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

void ZAliLoad::ShowTPCPlate(Int_t side, ZNode* top_holder)
{
  OpMutexHolder omh(this, "ShowTPCPlate()");
  ZGlass* cfg = 0;
  try {
    cfg = mQueen->FindLensByPath("Etc/Alice/TPC_RM_list");
  }
  catch(...) {}

  ZNode* holder;
  if( side == 0) 
    holder = new ZNode(GForm("%s TPCD front", mDataDir.Data()));
  else if (side == 1)
    holder = new ZNode(GForm("%s TPCD back", mDataDir.Data()));
  else 
    holder = new ZNode(GForm("%s TPCD ", mDataDir.Data()));
  
  if (side == 0 || side == -1) {
    for (int i=0; i < 18; i++) ShowTPCSegment(i, holder);
    if(cfg) holder->SetRnrMod(cfg);
  } 
  if ( side == 1 || side == -1) {
    mQueen->CheckIn(holder);
    for (int i=18; i < 2*18; i++) ShowTPCSegment(i, holder);
    if(cfg) holder->SetRnrMod(cfg);
  }

  if(top_holder == 0) top_holder = this;
  mQueen->CheckIn(holder);
  top_holder->Add(holder);
}

/**************************************************************************/

void ZAliLoad::check_itsdig_info()
{
  if (mITSDigInfo != 0 &&
      (mITSDigInfo->GetDataDir() != mDataDir || 
       mITSDigInfo->GetEvent()   != mEvent))
    { 
      mITSDigInfo->DecRefCount();
      mITSDigInfo = 0;
    }
  if (mITSDigInfo == 0)
    {
      mITSDigInfo = new ITSDigitsInfo();
      mITSDigInfo->IncRefCount();
      mITSDigInfo->SetData(mDataDir, mEvent);
    }
}

void ZAliLoad::ShowITSModule(Int_t id, Bool_t scale, ZNode* holder)
{
  static const string _eh("ZAliLoad::ShowITSModule ");

  check_itsdig_info();

  ITSModule* m;

  if(scale){
    m = new ITSScaledModule(id, mITSDigInfo);
  }
  else  
    m = new ITSModule(id, mITSDigInfo);

  mQueen->CheckIn(m);
  if (holder) 
    holder->Add(m);
  else Add(m);
}
 
void ZAliLoad::ShowITSDet(Int_t id, Bool_t scale, ZNode* holder, Bool_t show_empty)
{
  OpMutexHolder omh(this, "ShowITSDet");

  check_itsdig_info();
 
  Int_t layer, lad, det;
  ZNode *dh, *hl1, *hl2, *th;
  dh = hl1 = hl2 = 0;
  Int_t first, last;
  TClonesArray* arr;

  // top holder 
  if(id == 0) 
    dh = new ZNode("SPD");
  else if(id == 1)
    dh = new ZNode("SDD");
  else if(id == 2)
    dh = new ZNode("SSD");
  else
    dh = new ZNode("ITS Digits");



  if(id == -1 || id == 0) {
    hl1 = new ZNode("InnerLayer"); hl2 = new ZNode("OuterLayer");
    mQueen->CheckIn(hl1); mQueen->CheckIn(hl2);
    dh->Add(hl1); dh->Add(hl2);
   
    first = mITSDigInfo->mGeom->GetStartSPD();
    last = mITSDigInfo->mGeom->GetLastSPD();
    for(Int_t i =first; i<last; i++ ){
      arr= mITSDigInfo->GetDigits(i,0);
      mITSDigInfo->mGeom->GetModuleId(i,layer,lad, det);
      th = (layer == 1) ? hl1 : hl2;
      if(arr->GetEntriesFast() || show_empty)
	ShowITSModule(i,scale, th);
    }
  }
 
  if(id == -1 || id == 1) {
    hl1 = new ZNode("InnerLayer"); hl2 = new ZNode("OuterLayer");
    mQueen->CheckIn(hl1); mQueen->CheckIn(hl2);
    dh->Add(hl1); dh->Add(hl2);
 
    first = mITSDigInfo->mGeom->GetStartSDD();
    last = mITSDigInfo->mGeom->GetLastSDD();
    for(Int_t i =first; i<last; i++ ){
      arr= mITSDigInfo->GetDigits(i,1);
      mITSDigInfo->mGeom->GetModuleId(i,layer,lad, det);
      th = (layer == 3) ? hl1 : hl2;
      if(arr->GetEntriesFast() || show_empty)
	ShowITSModule(i, scale, th);
    }
  }
 
  if(id == -1 || id == 2) {
    hl1 = new ZNode("InnerLayer"); hl2 = new ZNode("OuterLayer");
    mQueen->CheckIn(hl1); mQueen->CheckIn(hl2);
    dh->Add(hl1); dh->Add(hl2);
 
    first = mITSDigInfo->mGeom->GetStartSSD();
    last = mITSDigInfo->mGeom->GetLastSSD();
    for(Int_t i =first; i<last; i++ ){
      arr= mITSDigInfo->GetDigits(i,2);
      mITSDigInfo->mGeom->GetModuleId(i,layer,lad, det);
      th = (layer == 5) ? hl1 : hl2;
      if(arr->GetEntriesFast() || show_empty)
	ShowITSModule(i, scale, th);
    }
  } 

  if(holder == 0) holder = this;
  mQueen->CheckIn(dh); holder->Add(dh);
}
/**************************************************************************/
void ZAliLoad::check_tofdig_info()
{
  if (mTOFDigInfo != 0 &&
      (mTOFDigInfo->GetDataDir() != mDataDir || 
       mTOFDigInfo->GetEvent()   != mEvent))
    { 
      mTOFDigInfo->DecRefCount();
      mTOFDigInfo = 0;
    }
  if (mTOFDigInfo == 0)
    {
      mTOFDigInfo = new TOFDigitsInfo();
      mTOFDigInfo->IncRefCount();
      mTOFDigInfo->SetData(mDataDir, mEvent);
    }
}


void ZAliLoad::ShowTOFSector(Int_t sec, ZNode* holder)
{
  if (holder == 0) holder = this;

  if(sec == -1) {
    ZNode* dh = new ZNode(GForm("%s TOF Digits", mDataDir.Data()));
    mQueen->CheckIn(dh); holder->Add(dh);
    for(Int_t s = 0; s<18; s++) {
      check_tofdig_info();
      TOFSector* m = new TOFSector(s, mTOFDigInfo);
      mQueen->CheckIn(m);
      dh->Add(m);
    }
  }
  else {
    check_tofdig_info();
    TOFSector* m = new TOFSector(sec, mTOFDigInfo);
    mQueen->CheckIn(m);
    holder->Add(m);
  }
}

/**************************************************************************/
/**************************************************************************/
// Wrappers
/**************************************************************************/

void ZAliLoad::SelectParticles(const Text_t* selection,Bool_t import_daughters)
{
  static const string _eh("ZAliLoad::SelectParticles ");

  OpMutexHolder omh(this, "SelectParticles");

  if(!mSelector)
    throw(_eh + "No VSD data loaded.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mParticleSelection.Data();

  mSelector->SelectParticles(0, selection, import_daughters);
}
/**************************************************************************/

void ZAliLoad::SelectHits(const Text_t* selection)
{
  static const string _eh("ZAliLoad::SelectHits ");

  OpMutexHolder omh(this, "SelectHits");

  if(!mSelector)
    throw(_eh + "No VSD data loaded.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mHitSelection.Data();

  mSelector->SelectHits(0, selection);
}
/**************************************************************************/

void ZAliLoad::SelectClusters(const Text_t* selection)
{
  static const string _eh("ZAliLoad::SelectClusters ");

  OpMutexHolder omh(this, "SelectClusters");

  if(!mSelector)
    throw(_eh + "No VSD data loaded.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mClusterSelection.Data();

  mSelector->SelectClusters(0, selection);
}
/**************************************************************************/

void ZAliLoad::SelectRecTracks(const Text_t* selection)
{
  static const string _eh("ZAliLoad::SelectRecTracks ");

  OpMutexHolder omh(this, "SelectRecTracks");

  if(!mSelector)
    throw(_eh + "No VSD data loaded.");

 if(selection == 0 || strcmp(selection,"") == 0)
    selection = mRecSelection.Data();

  mSelector->SelectRecTracks(0, selection);
}
/**************************************************************************/

void ZAliLoad::SelectV0(const Text_t* selection, Bool_t import_kine)
{
  static const string _eh("ZAliLoad::SelectV0 ");

  OpMutexHolder omh(this, "SelectV0 ");

  if(!mSelector)
    throw(_eh + "No VSD data loaded.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mV0Selection.Data();

  mSelector->SelectV0(0, selection, import_kine);
}
/**************************************************************************/

void ZAliLoad::SelectKinks(const Text_t* selection, Bool_t import_kine, Bool_t import_daughters)
{
  static const string _eh("ZAliLoad::SelectKinks ");

  OpMutexHolder omh(this, "SelectKinks ");

  if(!mSelector)
    throw(_eh + "No VSD data loaded.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mKinkSelection.Data();

  mSelector->SelectKinks(0, selection, import_kine, import_daughters);
}
/**************************************************************************/

void ZAliLoad::SelectGenInfo(const Text_t* selection)
{
  static const string _eh("ZAliLoad::SelectGenInfo ");

  OpMutexHolder omh(this, "SelectGenInfo");

  if(!mSelector)
    throw(_eh + "No VSD data loaded.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mGISelection.Data();

  mSelector->SelectGenInfo(0, selection);
}


