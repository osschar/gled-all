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
  ///printf("Write Converter in VSD file %s , data dir %s\n",mVSDFile.Data(),mDataDir.Data() );
  mConverter->CreateVSD(mDataDir, mVSDFile);
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

  SetDataDir(".");
  if(m_auto_vsdfile_p = true)
    SetVSDFile("");
}

/**************************************************************************/
// DIGITS
/**************************************************************************/

TPCSegment* ZAliLoad::ShowTPCSegment(Int_t segment_id, ZNode* holder)
{
  static const string _eh("ZAliLoad::ShowTPCSegment ");

  if (mTPCDigInfo == 0 || (mTPCDigInfo->GetDataDir() != mDataDir)) {
    mTPCDigInfo = new TPCDigitsInfo();
    mTPCDigInfo->SetData(mDataDir);
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
  OpMutexHolder omh(this, "ShowTPCPlate()");
  ZGlass* cfg = 0;
  try {
    cfg = mQueen->FindLensByPath("Etc/Alice/TPC_RM_list");
  }
  catch(...) {}

  if (side == 0 || side == -1) {
    ZNode* holder = new ZNode(GForm("%s TPCD front", mDataDir.Data()));
    mQueen->CheckIn(holder);
    for (int i=0; i < 18; i++) ShowTPCSegment(i, holder);
    Add(holder);
    if(cfg) holder->SetRnrMod(cfg);
  } 
  if ( side == 1 || side == -1) {
    ZNode* holder = new ZNode(GForm("%s TPCD back", mDataDir.Data()));
    mQueen->CheckIn(holder);
    for (int i=18; i < 2*18; i++) ShowTPCSegment(i, holder);
    Add(holder);
    if(cfg) holder->SetRnrMod(cfg);
  }
}

/**************************************************************************/

void ZAliLoad::ShowITSModule(Int_t id, ZNode* holder)
{
  static const string _eh("ZAliLoad::ShowITSModule ");

  if (mITSDigInfo == 0 || (mITSDigInfo->GetDataDir() != mDataDir)) {
    mITSDigInfo = new ITSDigitsInfo();
    mITSDigInfo->SetData(mDataDir);
  }

  ITSModule* m = new ITSModule(id, mITSDigInfo);
  mQueen->CheckIn(m);
  if (holder) 
    holder->Add(m);
  else Add(m);
}
 
void ZAliLoad::ShowITSDet(Int_t id, Bool_t show_empty)
{
  OpMutexHolder omh(this, "ShowITSDet()");

  if (mITSDigInfo == 0 || (mITSDigInfo->GetDataDir() != mDataDir)) {
    mITSDigInfo = new ITSDigitsInfo();
    mITSDigInfo->SetData(mDataDir);
  }
 
  Int_t layer, lad, det;
  ZNode *dh, *hl1, *hl2, *th;
  Int_t first, last;
  TClonesArray* arr;
 
  if(id == -1 || id == 0){
    dh = new ZNode(GForm("%s SPD", mDataDir.Data()));
    mQueen->CheckIn(dh); Add(dh);
    hl1 = new ZNode("Layer 1"); hl2 = new ZNode("Layer 2");
    mQueen->CheckIn(hl1); mQueen->CheckIn(hl2);
    dh->Add(hl1); dh->Add(hl2);
   
    first = mITSDigInfo->mGeom->GetStartSPD();
    last = mITSDigInfo->mGeom->GetLastSPD();
    for(Int_t i =first; i<last; i++ ){
      arr= mITSDigInfo->GetDigits(i,0);
      mITSDigInfo->mGeom->GetModuleId(i,layer,lad, det);
      th = (layer == 1) ? hl1 : hl2;
      if(arr->GetEntriesFast() || show_empty)
	ShowITSModule(i, th);
    }
  }
 
  if(id == -1 || id == 1){
    dh = new ZNode(GForm("%s SDD", mDataDir.Data()));
    mQueen->CheckIn(dh); Add(dh);
    hl1 = new ZNode("Layer 3"); hl2 = new ZNode("Layer 4");
    mQueen->CheckIn(hl1); mQueen->CheckIn(hl2);
    dh->Add(hl1); dh->Add(hl2);
 
    first = mITSDigInfo->mGeom->GetStartSDD();
    last = mITSDigInfo->mGeom->GetLastSDD();
    for(Int_t i =first; i<last; i++ ){
      arr= mITSDigInfo->GetDigits(i,1);
      mITSDigInfo->mGeom->GetModuleId(i,layer,lad, det);
      th = (layer == 3) ? hl1 : hl2;
      if(arr->GetEntriesFast() || show_empty)
	ShowITSModule(i, th);
    }
  }
 
  if(id == -1 || id == 2){
    dh = new ZNode(GForm("%s SSD", mDataDir.Data()));
    mQueen->CheckIn(dh); Add(dh);
    hl1 = new ZNode("Layer 5"); hl2 = new ZNode("Layer 6");
    mQueen->CheckIn(hl1); mQueen->CheckIn(hl2);
    dh->Add(hl1); dh->Add(hl2);
 
    first = mITSDigInfo->mGeom->GetStartSSD();
    last = mITSDigInfo->mGeom->GetLastSSD();
    for(Int_t i =first; i<last; i++ ){
      arr= mITSDigInfo->GetDigits(i,2);
      mITSDigInfo->mGeom->GetModuleId(i,layer,lad, det);
      th = (layer == 5) ? hl1 : hl2;
      if(arr->GetEntriesFast() || show_empty)
	ShowITSModule(i, th);
    }
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


