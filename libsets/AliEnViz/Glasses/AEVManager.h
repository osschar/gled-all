// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVManager_H
#define AliEnViz_AEVManager_H

#include <Glasses/ZNameMap.h>

class SolarSystem;

class AEVSite;
class AEVSiteList;
class AEVMlSucker;

class AEVManager : public ZNameMap
{
  MAC_RNR_FRIENDS(AEVManager);

private:
  void _init();

public:
  enum SiteViz_e { SV_None, SV_All, SV_RunningJobs, SV_ErrorJobs };
  enum UserViz_e { UV_None, UV_All, UV_RunningJobs, UV_ErrorJobs };

protected:
  ZLink<SolarSystem>    mSolarSystem; // X{GS} L{A}
  ZLink<AEVSiteList>    mSites;       // X{GS} L{A}
  ZLink<AEVMlSucker>    mSucker;      // X{GS} L{A}

  SiteViz_e             mSiteVizMode; // X{GS} Ray{SiteViz} 7 PhonyEnum()
  Int_t                 mSiteMinVal;  // X{G}  7 ValOut(-join=>1)
  Int_t                 mSiteMaxVal;  // X{G}  7 ValOut()
  Int_t                 mSiteMinCut;  // X{GE} 7 Value(-range=>[0,10000, 1], -join=>1)
  Int_t                 mSiteMaxCut;  // X{GE} 7 Value(-range=>[0,10000, 1])

  UserViz_e             mUserVizMode; // X{GS} Ray{UserViz} 7 PhonyEnum()

#ifndef __CINT__
  typedef Int_t AEVSite::*SiteInt_t;

  struct SiteVar
  {
    SiteInt_t   fVarPtr;
    Int_t       fMin, fMax;
    Int_t       fLow, fHigh;

    SiteVar() : fVarPtr(0), fMin(0), fMax(0), fLow(0), fHigh(0) {}
  };

  SiteVar       mSVJobsRunning; //!
  SiteVar       mSVJobsError;   //!

  SiteVar      *mSVCurrent;     //!
#endif

  void update_sv_current();
  void select_sv_current();
  void update_sv_current(AEVSite* site);
  void select_sv_current(AEVSite* site);

public:
  AEVManager(const Text_t* n="AEVManager", const Text_t* t=0);
  virtual ~AEVManager();

  void EmitSiteVizRay();
  void EmitUserVizRay();

  void SetSiteMinCut(Int_t min);
  void SetSiteMaxCut(Int_t max);

  void SiteNew(AEVSite* site);
  void SitePositionChanged(AEVSite* site);
  void SiteChanged(AEVSite* site);

#include "AEVManager.h7"
  ClassDef(AEVManager, 1);
}; // endclass AEVManager

#endif
