// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AEVAlienUI
//
//

#include "AEVAlienUI.h"
#include "AEVAlienUI.c7"
#include "AEVSite.h"

#include <Glasses/ZQueen.h>
#include <Stones/ZComet.h>
#include <TFile.h>

ClassImp(AEVAlienUI)

/**************************************************************************/

void AEVAlienUI::_init()
{
  mSites = 0;
  bConnected = false;
  mBDP.init("AEVAlienUI");
}

AEVAlienUI::~AEVAlienUI()
{
  if(bConnected) {
    mBDP.close_fifos();
  }
  system(GForm("rm -rf %s", mBDP.fDir.Data()));
}

/**************************************************************************/

void AEVAlienUI::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  if(mSites == 0) {
    ZNameMap* l = new ZNameMap("AliEn Sites", GForm("Nests of %s", GetName()));
    l->SetElementFID(AEVSite::FID());
    l->SetMIRActive(false);
    mQueen->CheckIn(l);
    SetSites(l);
  }
}

/**************************************************************************/

void AEVAlienUI::Connect()
{
  static const Exc_t _eh("AEVAlienUI::Connect ");

  ZMIR* mir = suggest_MIR_presence(_eh, ZGlass::MC_IsFlare);

  if(IsSunOrFireSpace()) {
    if(bConnected) {
      throw(_eh + "already connected.");
    }

    GMutexHolder ali_lock(hAliEnLock);

    //int cmdret = system
    //  (GForm("aev_ui %s %s > %s/aev_log 2>&1 &",
    //     mBDP.fAtoBName.c_str(), mBDP.fBtoAName.c_str(), mBDP.fDir.c_str())
    //  );

    mBDP.open_BtoA();
    mBDP.read_line();
    if(strncmp(mBDP.fRB,"ERR",3) == 0) {
      mBDP.read_line();
      mBDP.close_fifos();
      if(mir) {
	throw(_eh + "initialization error: " + mBDP.fRB);
      } else {
	ISerr(_eh + "initialization error: " + mBDP.fRB);
	return;
      }
    }
    mBDP.open_AtoB();
  }

  bConnected = true;
  Stamp(FID());
}

void AEVAlienUI::Disconnect()
{
  static const Exc_t _eh("AEVAlienUI::Disconnect ");

  suggest_MIR_presence(_eh, ZGlass::MC_IsFlare);

  if(IsSunOrFireSpace()) {
    if(!bConnected) {
      throw(_eh + "not connected.");
    }

    GMutexHolder ali_lock(hAliEnLock);
    mBDP.close_fifos();
    // printf("fifos closed ...\n");
  }

  bConnected = false;
  Stamp(FID());
}

/**************************************************************************/

void AEVAlienUI::ImportSites(const Text_t* partition)
{
  static const Exc_t _eh("AEVAlienUI::ImportSites ");

  using namespace AEV_NS;

  alien_lock();
  int n = mBDP.send_command(GForm("dump_sites\n%s", partition ? partition : ""));
  if(n > 0) {
    // printf("Importing %d sites ...\n", n);
    for(int i=0; i<n; ++i) {
      AEVSite *_s = new AEVSite, &s = *_s;
      s.SetName(mBDP.read_line());
      s.SetLocation(mBDP.read_line());
      s.SetDomain(mBDP.read_line());
      // s.SetSiteId(mBDP.read_int());
      // s.SetMasterHostId(mBDP.read_int());
      s.SetLatitude(mBDP.read_double());
      s.SetLongitude(mBDP.read_double());

      if(s.mName.CompareTo(s.mLocation) == 0)
	s.SetTitle(s.mName);
      else
	s.SetTitle(GForm("%s@%s", s.mName.Data(), s.mLocation.Data()));

    ZNameMap* l = new ZNameMap("AliEn Sites", GForm("Nests of %s", GetName()));
    l->SetElementFID(AEVSite::FID());
    mQueen->CheckIn(_s);
    try {
      mSites->Add(_s);
    }
    catch(Exc_t& exc) {
      mQueen->RemoveLens(_s);
      ISerr(_eh + GForm("error inserting site %s (%s).", s.mName.Data(), exc.Data()));
    }

    }
  } else {
    printf("ERROR: %s\n", mBDP.fError.Data());
  }
  alien_unlock();
}

void AEVAlienUI::SaveSites(const Text_t* file)
{
  // OK ... this is messy ... need REAL root support.
  ZComet c("AEVSites");

  {
  AList::Stepper<> s(*mSites);
  while(s.step())
    c.AddTopLevel(*s, false, false, 0);
  }

  TFile f(file, "RECREATE");
  c.Write();
  f.Close();
}

void AEVAlienUI::LoadSites(const Text_t* file)
{
  static const Exc_t _eh("AEVAlienUI::LoadSites ");

  mSites->ClearList();

  TFile f(file, "READ");
  auto_ptr<ZComet> c( dynamic_cast<ZComet*>(f.Get("AEVSites")) );
  f.Close();
  if(c.get() == 0) {
    // This check should be more elaborate ... but need common infrastructure
    // anyway.
    throw(_eh + "could not read 'AEVSites' from file '" + file + "'.");
  }
  mQueen->AdoptComet(*mSites, 0, c.get());
}

/**************************************************************************/

void AEVAlienUI::FindExCountFilesPerSite(map<string,int>& cmap,
					 const Text_t* path,
					 const Text_t* wcard)
{
  static const Exc_t _eh("AEVAlienUI::FindEx ");
  using namespace AEV_NS;

  alien_lock();
  int n = mBDP.send_command(GForm("find_ex\n%s\n%s", path, wcard));
  if(n > 0) {
    printf("%s found %d files ...\n", _eh.Data(), n);
    for(int i=0; i<n; ++i) {
      string lfn(mBDP.read_line());
      string site(mBDP.read_line());
      cmap[site]++;
    }
  } else {
    if(mBDP.bError) {
      printf("ERROR: %s\n", mBDP.fError.Data());
    } else {
      printf("No matching files found.\n");
    }
  }
  alien_unlock();
}

/**************************************************************************/
