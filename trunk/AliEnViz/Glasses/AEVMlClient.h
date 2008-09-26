// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVMlClient_H
#define AliEnViz_AEVMlClient_H

#include <Glasses/ZNameMap.h>
#include <Gled/GTime.h>
#include <AliEnViz/AEV_NS.h>

class AEVMlClient : public ZNameMap
{
  MAC_RNR_FRIENDS(AEVMlClient);
  friend class AEVDemoDriver;

public:
  struct MonaEntry
  {
    TString fFarm, fCluster, fNode, fParam, fValue;
    GTime   fTime;
    TString fDateStr;

    virtual ~MonaEntry() {}
    const Text_t* StdFormat();

    ClassDef(MonaEntry, 1);
  };

  struct monaentry_time_lt
  {
    bool operator()(const MonaEntry& a, const MonaEntry& b) const
    { return a.fTime < b.fTime; }
  };

  struct monaentry_time_gt
  {
    bool operator()(const MonaEntry& a, const MonaEntry& b) const
    { return a.fTime > b.fTime; }
  };

  enum SortMode_e { SM_Ascending=-1, SM_None, SM_Descending };

private:
  void _init();
  GMutex  hMonaLock;

protected:
  TString mServer;    // X{GS} 7 Textor()
  TString mFarm;      // X{GS} 7 Textor()
  TString mCluster;   // X{GS} 7 Textor()
  TString mNode;      // X{GS} 7 Textor()
  TString mParam;     // X{GS} 7 Textor()

  Float_t mFromHrs;   // X{GS} 7 Value(-range=>[-1e8,1e8,1,1000], -join=>1)
  Float_t mToHrs;     // X{GS} 7 Value(-range=>[-1e8,1e8,1,1000])

  SortMode_e mFLSort; // X{GS} 7 PhonyEnum()

  Bool_t  bConnected; // X{GS} 7 BoolOut()

  AEV_NS::BiDiPipe mBDP;  //!
  void check_connection(const Exc_t& eh);

public:
  AEVMlClient(const Text_t* n="AEVMlClient", const Text_t* t=0) :
    ZNameMap(n,t),  hMonaLock(GMutex::recursive)
  { _init(); }
  virtual ~AEVMlClient() {}

  void OpenConn();  // X{E} 7 MButt(-join=>1)
  void CloseConn(); // X{E} 7 MButt()

  void GetValues(list<AEVMlClient::MonaEntry>& results);
  void GetFLValues(list<AEVMlClient::MonaEntry>& results);

  void PrintValues();   //! X{Ed} 7 MButt()
  void PrintFLValues(); //! X{Ed} 7 MButt()

  void FindJobs();      //  X{Ed} 7 MButt()
  void FakeFindJobs();  //  X{Ed}

#include "AEVMlClient.h7"
  ClassDef(AEVMlClient, 1);
}; // endclass AEVMlClient


#endif
