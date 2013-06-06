// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AEVMlSucker.h"
#include "AEVManager.h"
#include "AEVMlSucker.c7"

#include "AEVMapViz.h"
#include "AEVSite.h"
#include "AEVSiteList.h"

#include <Gled/GThread.h>

#include <TMath.h>
#include <TSystem.h>
#include <TSocket.h>

// AEVMlSucker

//__________________________________________________________________________
//
//

ClassImp(AEVMlSucker);

/**************************************************************************/

namespace
{
bool s_map_init_done = false;

// Site Int_t and Float_t setters

typedef void (AEVSite::*site_set_int_foo)  (Int_t);
typedef void (AEVSite::*site_set_float_foo)(Float_t);

map<TString, site_set_int_foo>     s_site_set_int_map;
map<TString, site_set_float_foo>   s_site_set_float_map;

void call_foo(AEVSite* site, const TString& key, Int_t val)
{
  site_set_int_foo foo = s_site_set_int_map[key];
  if (foo == 0)
    throw(Exc_t("site_set_int - unknown key '") + key + "'.");

  // printf("Calling setter for key '%s'\n", key.Data());
  GLensWriteHolder lock(site);
  (site->*foo)(val);
}

void call_foo(AEVSite* site, const TString& key, Float_t val)
{
  site_set_float_foo foo = s_site_set_float_map[key];
  if (foo == 0)
    throw(Exc_t("site_set_float - unknown key '") + key + "'.");

  // printf("Calling setter for key '%s'\n", key.Data());
  GLensWriteHolder lock(site);
  (site->*foo)(val);
}

}

void AEVMlSucker::_init()
{
  // from local dump: "cat nc-dump"
  mSuckHost  = "pcalimonitor2.cern.ch";
  mSuckPort  = 7014;
  mFooSleep = 0;

  mSuckerThread = 0;

  mGlobWordRE.Reset("([-\\w\\.]+)\\s+", "g");
  mGlobVarRE .Reset("([-_\\w]+)=([\\d\\.eE]+)(?:[,\\s]*)", "g");
  mUScoreRE  .Reset("(\\w+)_([-\\w]+)");

  mSock = 0;

  if ( ! s_map_init_done)
  {
    s_site_set_int_map["max_RUNNING"]    = &AEVSite::SetSiteSize;

    s_site_set_int_map["jobs_STARTED"]   = &AEVSite::SetJobsStarted;
    s_site_set_int_map["jobs_RUNNING"]   = &AEVSite::SetJobsRunning;
    s_site_set_int_map["jobs_SAVING"]    = &AEVSite::SetJobsSaving;
    s_site_set_int_map["jobs_DONE"]      = &AEVSite::SetJobsDone;
    s_site_set_int_map["jobs_ERROR_ALL"] = &AEVSite::SetJobsError;
    s_site_set_int_map["jobs_ERR"]       = &AEVSite::SetJobsError;

    s_site_set_int_map["totevents"]      = &AEVSite::SetEventsAll;
    s_site_set_int_map["events"]         = &AEVSite::SetEventsDone;

    s_site_set_float_map["geo_LAT"] = &AEVSite::SetLatitude;
    s_site_set_float_map["geo_LON"] = &AEVSite::SetLongitude;
  }
}

//==============================================================================

TString AEVMlSucker::next_word(const TString& s)
{
  if (mGlobWordRE.Match(s) != 2)
  {
    throw (Exc_t("Failed getting next word."));
  }

  return mGlobWordRE[1];
}

bool AEVMlSucker::next_var(const TString& s, TString& var, TString& val)
{
  if (mGlobVarRE.Match(s) != 3)
  {
    return false;
  }

  var = mGlobVarRE[1];
  val = mGlobVarRE[2];

  return true;
}

bool AEVMlSucker::next_var(const TString& s, TString& var, Float_t& val)
{
  TString v;
  bool ret = next_var(s, var, v);
  val = v.Atof();
  return ret;
}

//==============================================================================

void* AEVMlSucker::tl_Suck(AEVMlSucker* s)
{
  s->Suck();
  delete s->mSock; s->mSock = 0;
  s->mSuckerThread = 0;
  return 0;
}

void AEVMlSucker::Suck()
{
  static const Exc_t _eh("AEVMlSucker::Suck ");

  mSock = new TSocket(mSuckHost, mSuckPort);
  if (mSock->IsValid() == kFALSE)
  {
    printf("%sfailed opening the socket.\n", _eh.Data());
    return;
  }

  const Int_t buf_size = 1024;
  char        buf[buf_size];

  Int_t count = 0;

  while (true)
  {
    {
      Int_t len = 0;
      while (1)
      {
	Int_t ret = mSock->RecvRaw(&buf[len], 1);
	if (ret < 0) {
	  printf("%sError sucking %d.\n", _eh.Data(), ret);
	  mSock->Close();
	  return;
	}
	if (buf[len] == 10) {
	  buf[len] = 0;
	  break;
	}
	++len;
      }
    }

    TString l(buf);

    mGlobWordRE.ResetGlobalState();

    ++count;

    // printf("%-4d: '%s'\n", count, l.Data());

    try
    {
      TString l1 = next_word(l);

      if (l1 == "site")
      {
	TString name = next_word(l);

	AEVSiteList *sites  = find_lens<AEVSiteList>("Sites");

	bool new_site = false;
	AEVSite *site = sites->FindSite(name);
	if (site == 0)
	{
	  printf("New site '%s'.\n", name.Data());
	  new_site = true;
	  site = new AEVSite(name);
	}

	mGlobVarRE.AssignGlobalState(mGlobWordRE);
	TString var;
	Float_t val;
	Bool_t   reposition = false;
	while (next_var(l, var, val))
	{
	  if (var.BeginsWith("geo_"))
	  {
	    call_foo(site, var, val);
	    reposition = true;
	  }
	  else if (var.BeginsWith("jobs_") || var.BeginsWith("max_") || var.Contains("events"))
	  {
	    call_foo(site, var, TMath::Nint(val));
	  }
	  else
	  {
	    // printf("site - unhandled variable '%s'.\n", var.Data());
	  }
	}

	if (new_site)
	{
	  // !!! For cluster op, should IncarnateWAttach();
	  GLensWriteHolder wrlck(sites);

	  mQueen->CheckIn(site);
	  sites->Add(site);
	}

	assert_manager(_eh);

	GLensWriteHolder wrlck(*mManager);

	if (new_site)
	  mManager->SiteNew(site);
	if (reposition)
	  mManager->SitePositionChanged(site);
	mManager->SiteChanged(site);
      }
    }
    catch(Exc_t exc)
    {
      printf("Error sucking line %d: '%s'\n  Exception='%s'\n", count, l.Data(), exc.Data());
    }

    if (mFooSleep > 0)
      gSystem->Sleep(mFooSleep);
  }
}

void AEVMlSucker::StartSucker()
{
  static const Exc_t _eh("AEVMlSucker::StartSucker ");

  if (mSuckerThread)
    throw (_eh + "already running.");


  mSuckerThread = new GThread("AEVMlSucker-Sucker",
			      (GThread_foo) tl_Suck, this,
			      false);
  mSuckerThread->SetNice(20);
  mSuckerThread->Spawn();
}

void AEVMlSucker::StopSucker()
{
  static const Exc_t _eh("AEVMlSucker::StopSucker ");

  if (mSuckerThread == 0)
    throw (_eh + "not running.");

  mSuckerThread->Cancel();
  mSuckerThread->Join();
  mSuckerThread = 0;
}

//==============================================================================

void AEVMlSucker::SendLine(const TString& cmd)
{
  static const Exc_t _eh("AEVMlSucker::SendLine ");

  if (mSock == 0)
    throw(_eh + "socket not open.");

  Int_t ret = mSock->SendRaw(cmd.Data(), cmd.Length());
  if (ret < 0)
  {
    printf("%serror sending command %d.\n", _eh.Data(), ret);
  }
}
