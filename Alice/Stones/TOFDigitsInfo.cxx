// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TOFDigitsInfo
//
//

#include "TOFDigitsInfo.h"

#include <Gled/GledTypes.h>

#include <AliTOFdigit.h>

ClassImp(TOFDigitsInfo)

/**************************************************************************/

  void TOFDigitsInfo::_init()
{
  mDataDir = ".";
  mTree = 0;
  mGeom = 0;
  mDigits = 0;
}

/**************************************************************************/
void TOFDigitsInfo::SetData(const Text_t* data_dir, Int_t event)
{
  static const Exc_t _eh("TOFDigitsInfo::SetData ");

  mDataDir = "";
  mEvent   = -1;

  TFile* file = TFile::Open(GForm("%s/galice.root", data_dir));
  if(!file)
    throw(_eh + "galice.root file not found.");

  mGeom = (AliTOFGeometry*) file->Get("TOFGeometry");
  if (!mGeom){
    throw(_eh + "TOF Geometry not found");
  }
  mGeom->Dump();


  TFile* f2 = TFile::Open(GForm("%s/TOF.Digits.root", data_dir));
  if(f2 == 0)
    throw(_eh + "can not open TOF.Digits.root.");
 
  const Text_t* ev_dir = GForm("Event%d", event);
  TDirectory* d = (TDirectory*)f2->Get(ev_dir);
  if(d == 0)
    throw(_eh + "can not get directory '"+ ev_dir +"'.");
  mTree = (TTree*)d->Get("TreeD");

  //TClonesArray *arr = 0;
  mTree->SetBranchAddress("TOF", &mDigits);
  mTree->GetEntry(0);
 
  mDataDir = data_dir;
  mEvent   = event;
}

  /**************************************************************************/
