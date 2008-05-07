// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AEVMlClient
//
//

#include "AEVMlClient.h"
#include "AEVMlClient.c7"

#include "AEVJobRep.h"

#include <Glasses/ZQueen.h>

#include <TSystem.h>
#include <TRandom.h>

#include <sys/time.h>

typedef list<AEVMlClient::MonaEntry>           lMonaEntry_t;
typedef list<AEVMlClient::MonaEntry>::iterator lMonaEntry_i;

/**************************************************************************/

ClassImp(AEVMlClient::MonaEntry);

/**************************************************************************/

const Text_t* AEVMlClient::MonaEntry::StdFormat()
{
  return GForm("/%s/%s/%s/%-32s %12s (%s)", fFarm.Data(), fCluster.Data(),
	       fNode.Data(), fParam.Data(), fValue.Data(), fDateStr.Data());
}

/**************************************************************************/
/**************************************************************************/

ClassImp(AEVMlClient);

/**************************************************************************/

void AEVMlClient::_init()
{
  mServer  = "http://pcaliense01.cern.ch:6004/axis/services/MLWebService";
  mFarm    = "ALICE-SC05";
  mCluster = "ALIEN_QUERY";
  mNode    = "*";
  mParam   = "*";

  mFromHrs = 1;
  mToHrs   = 0;

  mFLSort  = SM_None;

  bConnected = false;
  mBDP.init("MlClient");
  // mBDP.bDebug = true;

  FILE* cpath = gSystem->OpenPipe("aev-monaclasspath.sh", "r");
  char buf[1024]; fgets(buf, 1024, cpath);
  int s = strlen(buf);
  if(s && buf[s-1] == 10) buf[s-1] = 0;
  setenv("CLASSPATH", buf, 1);
  gSystem->ClosePipe(cpath);
}

/**************************************************************************/
// This was used with perl client.

void AEVMlClient::OpenConn()
{
  static const Exc_t _eh("AEVMlClient::OpenConn ");

  if(bConnected) {
    throw(_eh + "already connected.");
  }

  GMutexHolder mllck(hMonaLock);

  // Perl client
  //   system(GForm("mona_client.pl %s %s %s > %s/ml_log 2>&1 &",
  // 	       mServer.Data(),
  // 	       mBDP.fAtoBName.c_str(), mBDP.fBtoAName.c_str(),
  // 	       mBDP.fDir.c_str())
  // 	 );

  // Java client
  system(GForm("java MonaClient %s %s %s > %s/ml_log 2>&1 &",
	       mServer.Data(),
	       mBDP.fAtoBName.c_str(), mBDP.fBtoAName.c_str(),
	       mBDP.fDir.c_str())
	 );

  mBDP.open_BtoA();
  mBDP.read_line();
  if(strncmp(mBDP.fRB,"OK",2) != 0) {
    mBDP.read_line();
    mBDP.close_fifos();
    throw(_eh + "initialization error: " + mBDP.fRB);
  }
  mBDP.open_AtoB();
  bConnected = true;
  Stamp(FID());
}

void AEVMlClient::CloseConn()
{
  static const Exc_t _eh("AEVMlClient::CloseConn ");

  if(!bConnected) {
    throw(_eh + "not connected.");
  }
  {
    GMutexHolder mllck(hMonaLock);
    mBDP.close_fifos();
    // printf("fifos closed ...\n");
  }
  bConnected = false;
  Stamp(FID());
}

void AEVMlClient::check_connection(const Exc_t& eh)
{
  if(bConnected == false)
    OpenConn();
  if(bConnected == false)
    throw(eh + "can not open connection.");
}

/**************************************************************************/

void AEVMlClient::GetValues(list<MonaEntry>& results)
{
  static const Exc_t _eh("AEVMlClient::GetValues ");

  check_connection(_eh);

  char tstr[26];
  long long int t_to   = (long long int)(-3600000*mToHrs);
   long long int t_from = (long long int)(-3600000*mFromHrs);
  // Nasty hack for Seattle time difference
  //long long int t_to   = (long long int)(-3600000*(mToHrs   - 24));
  //long long int t_from = (long long int)(-3600000*(mFromHrs + 24));

  GMutexHolder mllck(hMonaLock);

  int n = mBDP.send_command
    ( GForm("GetValues\n%s\n%s\n%s\n%s\n%lld\n%lld",
	    mFarm.Data(), mCluster.Data(), mNode.Data(), mParam.Data(),
	    t_from, t_to));

  if(n >= 0) {
    for(int i=0; i<n; ++i) {
      MonaEntry m;
      m.fFarm    = mBDP.read_line();
      m.fCluster = mBDP.read_line();
      m.fNode    = mBDP.read_line();
      m.fParam   = mBDP.read_line();
      m.fValue   = mBDP.read_line();

      time_t epochtime = time_t(atoll(mBDP.read_line())/1000);
      m.fTime.SetSec(epochtime);
      ctime_r(&epochtime, tstr);
      tstr[24]='\0';
      m.fDateStr = tstr;

      results.push_back(m);
    }
  } else {
    throw(_eh + "ERROR: " + mBDP.fError + ".");
  }
}

void AEVMlClient::GetFLValues(list<MonaEntry>& results)
{
  static const Exc_t _eh("AEVMlClient::GetFLValues ");

  check_connection(_eh);

  char tstr[26];

  GMutexHolder mllck(hMonaLock);

  int n = mBDP.send_command
    ( GForm("GetFLValues\n%s\n%s\n%s\n%s",
	    mFarm.Data(), mCluster.Data(), mNode.Data(), mParam.Data()) );

  if(n >= 0) {
    for(int i=0; i<n; ++i) {
      MonaEntry m;
      m.fFarm    = mBDP.read_line();
      m.fCluster = mBDP.read_line();
      m.fNode    = mBDP.read_line();
      m.fParam   = mBDP.read_line();
      m.fValue   = mBDP.read_line();

      time_t epochtime = time_t(atoll(mBDP.read_line())/1000);
      m.fTime.SetSec(epochtime);
      ctime_r(&epochtime, tstr);
      tstr[24]='\0';
      m.fDateStr = tstr;

      results.push_back(m);
    }

    switch (mFLSort) {
    case SM_Ascending:  results.sort(monaentry_time_lt()); break;
    case SM_Descending: results.sort(monaentry_time_gt()); break;
    default:            break;
    }

  } else {
    throw(_eh + "ERROR: " + mBDP.fError + ".");
  }
}

/**************************************************************************/

void AEVMlClient::PrintValues()
{
  static const Exc_t _eh("AEVMlClient::PrintValues ");

  list<MonaEntry> mes;
  GetValues(mes);

  int n = mes.size();
  printf("%s entries=%d\n", _eh.Data(), n);

  for(lMonaEntry_i i=mes.begin(); i!=mes.end(); ++i) {
    MonaEntry& m = *i;
    printf("/%s/%s/%s/%-32s %12s (%s)\n", m.fFarm.Data(), m.fCluster.Data(),
	   m.fNode.Data(), m.fParam.Data(), m.fValue.Data(), m.fDateStr.Data());
  }

}

void AEVMlClient::PrintFLValues()
{
  static const Exc_t _eh("AEVMlClient::PrintFLValues ");

  list<MonaEntry> mes;
  GetFLValues(mes);

  int n = mes.size();
  printf("%s entries=%d\n", _eh.Data(), n);

  for(lMonaEntry_i i=mes.begin(); i!=mes.end(); ++i) {
    MonaEntry& m = *i;
    printf("/%s/%s/%s/%-32s %12s (%s)\n", m.fFarm.Data(), m.fCluster.Data(),
	   m.fNode.Data(), m.fParam.Data(), m.fValue.Data(), m.fDateStr.Data());
  }
}

/**************************************************************************/

void AEVMlClient::FindJobs()
{
  static const Exc_t _eh("AEVMlClient::FindJobs ");

  GMutexHolder mllck(hMonaLock);

  { GLensWriteHolder wrlck(this);
    ClearList();

    mCluster = "ALIEN_PROCESS"; // "ALIEN_QUERY"; // "ALIEN_INFO";
    mNode    = "*";
    mParam   = "status";
    mFLSort  = SM_Descending;
    Stamp(FID());
  }

  list<MonaEntry> jobs;
  GetFLValues(jobs);

  { GLensWriteHolder wrlck(this);
    for(list<MonaEntry>::iterator i=jobs.begin(); i!=jobs.end(); ++i) {
      AEVJobRep* job = new AEVJobRep;

      job->SetName(i->fNode);
      job->SetStatus(i->fValue);
      job->SetDateStr(i->fDateStr);

      mQueen->CheckIn(job);
      Add(job);
    }
    mCluster = "ALIEN_QUERY";
    mParam   = "jobname";
  }

  jobs.clear(); GetFLValues(jobs);

  { GLensWriteHolder wrlck(this);
    for(list<MonaEntry>::iterator i=jobs.begin(); i!=jobs.end(); ++i) {
      AEVJobRep* job = (AEVJobRep*) GetElementByName(i->fNode);
      if(job == 0) {
	printf("%s (jobname) job '%s' not found!\n", _eh.Data(), i->fNode.Data());
	continue;
      }
      job->SetJobname(i->fValue);
    }
    mParam = "user";
  }

  jobs.clear(); GetFLValues(jobs);

  { GLensWriteHolder wrlck(this);
    for(list<MonaEntry>::iterator i=jobs.begin(); i!=jobs.end(); ++i) {
      AEVJobRep* job = (AEVJobRep*) GetElementByName(i->fNode);
      if(job == 0) {
	printf("%s (username) job '%s' not found!\n", _eh.Data(), i->fNode.Data());
	continue;
      }
      job->SetUsername(i->fValue);
    }

    Stepper<AEVJobRep> s(this);
    while(s.step())
      s->FormatTitle();

    Stamp(FID());
  }
}

namespace {

char* Jobnames[] = { "Anal-PiK", "Cal-QvsZ", "Fed-MacTest", "test", "CPU-burner" };
Int_t JobnamesN  = 5;

char* Usernames[] = { "apeters", "carminati", "alimaster", "feynmann", "buncic" };
Int_t UsernamesN  = 5;

char* Statuses[] = { "FINISHED", "FINISHED", "FINISHED", "RUNNING", "QUEUED", "WAITING" };
Int_t StatusesN  = 6;
}

void AEVMlClient::FakeFindJobs()
{
  Int_t n_fake = 35;
  Int_t n_days = 10;

  TRandom gen(0);

  vector<time_t> times(n_fake);
  GTime now(GTime::I_Now);
  for(Int_t n=0; n<n_fake; ++n) {
    times[n] = time_t(now.GetSec() - n_days*3600*24*gen.Rndm());
  }
  sort(times.begin(), times.end(), greater_equal<time_t>());

  { GLensWriteHolder wrlck(this);
    ClearList();

    char tstr[26];
    for(Int_t n=0; n<n_fake; ++n) {
      AEVJobRep* job = new AEVJobRep;

      job->SetName(GForm("Aid-%06d", Int_t(1e6*gen.Rndm())));
      job->SetJobname(Jobnames[Int_t(JobnamesN*gen.Rndm())]);
      job->SetUsername(Usernames[Int_t(UsernamesN*gen.Rndm())]);
      job->SetStatus(Statuses[Int_t(StatusesN*gen.Rndm())]);
      ctime_r(&times[n], tstr);
      job->SetDateStr(tstr);
      job->FormatTitle();

      mQueen->CheckIn(job);
      Add(job);

    }
  }
}
