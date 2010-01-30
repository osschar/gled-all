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

#include <TMath.h>
#include <TSystem.h>

ClassImp(ZAliLoad);

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
	throw(Exc_t(GForm("%s rejected (command queuing not implemented).", op)));
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
  mDataDir  = ".";
  mEvent    = 0;

  mDefVSDName = "AliVSD.root";
  mVSDFile    = "";

  mParticleSelection = "fStatusCode <= 1 && Pt() > 0.5";
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

Bool_t ZAliLoad::check_read(const TString& file)
{
  return (gSystem->AccessPathName(file, kReadPermission) == false);
}

TString ZAliLoad::get_vsd_name(Bool_t check_p)
{
  m_auto_vsdfile_p = false;

  TString file(mVSDFile);
  if(file != "") {
    if(!check_p || (check_p && check_read(file))) {
      return file;
    }
  }

  m_auto_vsdfile_p = true;

  TString there(GForm("%s/%s", mDataDir.Data(), mDefVSDName.Data()));
  if(!check_p || (check_p && check_read(there)))
    return there;

  TString here(mDefVSDName);
  if(!check_p || (check_p && check_read(here)))
    return here;

  throw(Exc_t(file + " | " + there + " | " + here));
}

/**************************************************************************/

void ZAliLoad::LoadVSD()
{
  static const Exc_t _eh("ZAliLoad::LoadVSD ");

  OpMutexHolder omh(this, "LoadVSD");

  TString vsd_file(get_vsd_name(true));
  SetVSDFile(vsd_file);

  mSelector = new VSDSelector(GForm("VSD %s", mDataDir.Data()));
  mQueen->CheckIn(*mSelector); Add(*mSelector);
  mSelector->LoadVSD(mVSDFile);
}

/**************************************************************************/

void ZAliLoad::ClearData()
{
  OpMutexHolder omh(this, "ResetData()");
 
  SetSelector(0);
  RemoveLensesViaQueen(true);
 
  SetDataDir(".");
  if(m_auto_vsdfile_p)
    SetVSDFile("");
}

/**************************************************************************/
/**************************************************************************/
// Wrappers
/**************************************************************************/

void ZAliLoad::SelectParticles(const Text_t* selection,Bool_t import_daughters)
{
  static const Exc_t _eh("ZAliLoad::SelectParticles ");

  OpMutexHolder omh(this, "SelectParticles");

  if(mSelector == 0)
    throw(_eh + "No VSD data loaded.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mParticleSelection.Data();

  mSelector->SelectParticles(0, selection, import_daughters);
}
/**************************************************************************/

void ZAliLoad::SelectHits(const Text_t* selection)
{
  static const Exc_t _eh("ZAliLoad::SelectHits ");

  OpMutexHolder omh(this, "SelectHits");

  if(mSelector == 0)
    throw(_eh + "No VSD data loaded.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mHitSelection.Data();

  mSelector->SelectHits(0, selection);
}
/**************************************************************************/

void ZAliLoad::SelectClusters(const Text_t* selection)
{
  static const Exc_t _eh("ZAliLoad::SelectClusters ");

  OpMutexHolder omh(this, "SelectClusters");

  if(mSelector == 0)
    throw(_eh + "No VSD data loaded.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mClusterSelection.Data();

  mSelector->SelectClusters(0, selection);
}
/**************************************************************************/

void ZAliLoad::SelectRecTracks(const Text_t* selection)
{
  static const Exc_t _eh("ZAliLoad::SelectRecTracks ");

  OpMutexHolder omh(this, "SelectRecTracks");

  if(mSelector == 0)
    throw(_eh + "No VSD data loaded.");

 if(selection == 0 || strcmp(selection,"") == 0)
    selection = mRecSelection.Data();

  mSelector->SelectRecTracks(0, selection);
}
/**************************************************************************/

void ZAliLoad::SelectV0(const Text_t* selection, Bool_t import_kine)
{
  static const Exc_t _eh("ZAliLoad::SelectV0 ");

  OpMutexHolder omh(this, "SelectV0 ");

  if(mSelector == 0)
    throw(_eh + "No VSD data loaded.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mV0Selection.Data();

  mSelector->SelectV0(0, selection, import_kine);
}
/**************************************************************************/

void ZAliLoad::SelectKinks(const Text_t* selection, Bool_t import_kine, Bool_t import_daughters)
{
  static const Exc_t _eh("ZAliLoad::SelectKinks ");

  OpMutexHolder omh(this, "SelectKinks ");

  if(mSelector == 0)
    throw(_eh + "No VSD data loaded.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mKinkSelection.Data();

  mSelector->SelectKinks(0, selection, import_kine, import_daughters);
}
/**************************************************************************/

void ZAliLoad::SelectGenInfo(const Text_t* selection)
{
  static const Exc_t _eh("ZAliLoad::SelectGenInfo ");

  OpMutexHolder omh(this, "SelectGenInfo");

  if(mSelector == 0)
    throw(_eh + "No VSD data loaded.");

  if(selection == 0 || strcmp(selection,"") == 0)
    selection = mGISelection.Data();

  mSelector->SelectGenInfo(0, selection);
}


