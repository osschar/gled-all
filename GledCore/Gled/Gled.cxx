// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Gled.h"
#include <Gled/GledNS.h>
#include <Gled/GMutex.h>
#include <Gled/GCondition.h>
#include <Ephra/Saturn.h>
#include <Glasses/ZQueen.h>
#include <Glasses/SaturnInfo.h>
#include <Glasses/ShellInfo.h>

#include <glue/GledCore_LibSet.h>

#include <TSystem.h>
#include <TROOT.h>
#include <TSocket.h>
#include <TObjectTable.h>

ClassImp(Gled)

/**************************************************************************/

Gled* Gled::theOne = 0;

/**************************************************************************/

void Gled::next_arg_or_die(list<char*>& args, list<char*>::iterator& i)
{
  list<char*>::iterator j = i;
  if(++j == args.end()) {
    cerr <<"Option "<< *i <<" requires an argument\n";
    exit(1);
  }
  i = j;
}

/**************************************************************************/

Gled::Gled(list<char*>& args) : mSaturn(0), bIsSun(false),
				bQuit(false),
				bShowSplash(true), 
				bAutoSpawn(false),
				bAllowMoons(true),
				bRunRint(true)
{
  if(theOne) {
    cerr <<"Gled::Gled trying to instantiate another object ...\n";
    exit(1);
  }
  theOne = this;

  // Set-up SaturnInfo

  mSaturnInfo = new SaturnInfo;
  mSaturnInfo->SetHostName(gSystem->HostName());

  FILE* p = gSystem->OpenPipe("GledNodeReport.pl cpuinfo meminfo", "r");
  if(p != 0) {
    char buf[80]; UShort_t frq, num, mem, swp;
    fgets(buf, 80, p); char* nl = rindex(buf, 10); if(nl) *nl = 0;
    fscanf(p, "%hu %hu %hu %hu", &frq, &num, &mem, &swp);
    mSaturnInfo->SetCPU_Model(buf);
    mSaturnInfo->SetCPU_Freq(frq);
    mSaturnInfo->SetCPU_Num(num);
    mSaturnInfo->SetMemory(mem);
    mSaturnInfo->SetSwap(swp);
    gSystem->ClosePipe(p);
  }

  // Parse command-line options

  list<char*>::iterator i = args.begin();
  while(i != args.end()) {
    list<char*>::iterator start = i;

    if(strcmp(*i, "-h")==0 || strcmp(*i, "-help")==0 ||
       strcmp(*i, "--help")==0 || strcmp(*i, "-?")==0)
      {
	cout <<
	  "Arguments: [options] [dir] [file(s)]\n"
	  "			dir   ~ cd to dir prior to exec of files\n"
	  "			files ~ ROOT macro scripts to process\n"
	  "Gled options:\n"
	  "  -r[un]		spawn Saturn/Sun immediately\n"
	  "			Saturn if -master is specified, Sun otherwise\n"
	  "  -nomoons		do not accept moon connections\n"
	  "  -s[ssize]	<num>	specify size of sun-space (can be eg. 2e20)\n"
	  "  -p[ort]	<num>	specify server port (def 9061)\n"
	  "  -m[aster] <host>:<port> master Saturn address (def port 9061)\n"
	  "  -n[ame]	<str>	name of Saturn\n"
	  "  -t[itle]	<str>	title of Saturn\n"
	  "  -l			no splash info\n"
	  "  -norint		do not run TRint (useful for batch saturns)\n";
	bQuit = true;
	return;
      }

    else if(strcmp(*i, "-r")==0 || strcmp(*i, "-run")==0) {
      bAutoSpawn = true;
      args.erase(start, ++i);
    }

    else if(strcmp(*i, "-nomoons")==0) {
      bAllowMoons = false;
      args.erase(start, ++i);
    }

    else if(strcmp(*i, "-s")==0 || strcmp(*i, "-sssize")==0) {
      next_arg_or_die(args, i);
      if(index(*i, 'e')) {
	int m, e, num = sscanf(*i, "%de%d", &m, &e);
	if(num != 2) { cerr <<"-sssize poor exp format: "<< *i <<endl; exit(1); }
	mSaturnInfo->SetSunSpaceSize( ID_t(pow(double(m),e)) );
      } else {
	mSaturnInfo->SetSunSpaceSize( ID_t(atoll(*i)) );
      }
      args.erase(start, ++i);
    }

    else if(strcmp(*i, "-p")==0 || strcmp(*i, "-port")==0) {
      next_arg_or_die(args, i);
      mSaturnInfo->SetServerPort( atoi(*i) );
      args.erase(start, ++i);
    }

    else if(strcmp(*i, "-m")==0 || strcmp(*i, "-master")==0) {
      next_arg_or_die(args, i);
      char* col = index(*i, ':');
      if(col) {
	*(col++) = 0;
	UShort_t p = UShort_t(atoi(col));
	mSaturnInfo->SetMasterPort(p);
      }
      mSaturnInfo->SetMasterName(*i);
      args.erase(start, ++i);
    }

    else if(strcmp(*i, "-n")==0 || strcmp(*i, "-name")==0) {
      next_arg_or_die(args, i);
      mSaturnInfo->SetName(*i);
      args.erase(start, ++i);
    }

    else if(strcmp(*i, "-t")==0 || strcmp(*i, "-title")==0) {
      next_arg_or_die(args, i);
      mSaturnInfo->SetTitle(*i);
      args.erase(start, ++i);
    }

    else if(strcmp(*i, "-l")==0) {
      bShowSplash = false;
      args.erase(start, ++i);
    }

    else if(strcmp(*i, "-norint")==0) {
      bRunRint = false;
      args.erase(start, ++i);
    }

    else {
      ++i;
    }

  }

  if(bShowSplash) {
    cout <<
      "+----------------------------------------------------------+\n"
      "| This is Gled, version 1.1.9                              |\n"
      "| Gled is free software, released under GNU GPL version 2. |\n"
      "| For further information visit http://www.gled.org/       |\n"
      "+----------------------------------------------------------+\n"
      "Bootstraping ...\n";
  }

  libGledCore_GLED_init();
}

Gled::~Gled() {
  // Wipe saturn !!!!
}

/**************************************************************************/

void Gled::SpawnSunOrSaturn() {
  if(strcmp(mSaturnInfo->GetMasterName(), "") == 0) {
    mSaturnInfo->SetMasterPort(0);
    SpawnSun();
  } else {
    SpawnSaturn();
  }
}

void Gled::SpawnSun()
{
  if(mSaturn) return;
  if(strcmp(mSaturnInfo->GetName(), "SaturnInfo") == 0)
    mSaturnInfo->SetName(GForm("Sun at %s", gSystem->HostName()));
  mSaturn = new Saturn;
  mSaturn->Create(mSaturnInfo);
  bIsSun = true;
}

void Gled::SpawnSaturn()
{
  if(mSaturn) return;
  if(strcmp(mSaturnInfo->GetName(), "SaturnInfo") == 0)
    mSaturnInfo->SetName(GForm("Saturn at %s", gSystem->HostName()));
  mSaturn = new Saturn;
  try {
    SaturnInfo* si = mSaturn->Connect(mSaturnInfo);
    if(si) {
      delete mSaturnInfo;
      mSaturnInfo = si;
    } else {
      cerr <<"Gled::SpawnSaturn failed ... dying\n";
      exit(1);
    }
    WaitUntillQueensLoaded();
  }
  catch(string exc) {
    cerr <<"Gled::SpawnSaturn failed ... dying at:\n  "<< exc <<endl;
    exit(1);
  }
}

/**************************************************************************/

void Gled::WaitUntillQueensLoaded()
{
  // Wait until number of arriving queens is zero.

  mSaturn->RefQueenLoadCnd().Lock();
  while(1) {
    int n = mSaturn->GetQueenLoadNum();
    ISmess(GForm("Gled::WaitUntillQueensLoaded() #queens=%d", n));
    if(n==0) {
      mSaturn->RefQueenLoadCnd().Unlock();
      break;
    }
    mSaturn->RefQueenLoadCnd().Wait();
  }
}

void Gled::AllowMoonConnections()
{
  // Signal Saturn to allow connections by Moons.

  if(mSaturn) mSaturn->AllowMoons();
}

/**************************************************************************/
// Gled stuff
/**************************************************************************/

void Gled::Exit()
{
  if(mSaturn) mSaturn->Shutdown();
  bQuit = true;
  if(mExitCondVar) {
    mExitCondVar->Lock();
    mExitCondVar->Signal();
    mExitCondVar->Unlock();
  }
}

/**************************************************************************/

Int_t Gled::LoadLibSet(const Text_t* lib_set)
{
  return GledNS::LoadSoSet(lib_set);
}

Int_t Gled::LoadLibSet(LID_t lid)
{
  if(GledNS::IsLoaded(lid)) {
    return 0;
  } else {
    Text_t buf[80];
    FILE* f = fopen(GForm("%s/lib/.%u", getenv("GLEDSYS"), lid), "r");
    if(f == 0) {
      ISerr(GForm("Gled::LoadLibSet lid %u can not be demangled", lid));
      return -1;
    }
    fgets(buf, 80, f);
    fclose(f);
    int i = 0; while(i<79 && (isalnum(buf[i]) || buf[i]=='_')) {++i;} buf[i]=0;
    return LoadLibSet(buf);
  }
}

Int_t Gled::AssertLibSet(const Text_t* lib_set)
{
  if(GledNS::IsLoaded(lib_set)) {
    return 0;
  } else {
    return LoadLibSet(lib_set);
  }
}

/**************************************************************************/

void Gled::SetDebugLevel(Int_t d) {
  if(d<0) d=0;
  G_DEBUG = d;
}

/**************************************************************************/
// Info Stream methods
/**************************************************************************/

void Gled::output(const char* s) {
  cout << s << endl;
}

void Gled::message(const char* s) {
  cout <<"MSG: "<< s << endl;
}

void Gled::warning(const char* s) {
  cout <<"WARN: "<< s << endl;
}

void Gled::error(const char* s) {
  cerr << s << endl;
}

/**************************************************************************/

void Gled::SpawnEye(const char* name, const char* title)
{
  if(mSaturn == 0) {
    ISerr("Gled::SpawnEye Saturn has not been spawned");
    return;
  }

  ZQueen* fq = mSaturn->GetFireQueen();
  ShellInfo *si = new ShellInfo("Generick Shell", "created by Gled::SpawnEye");
  fq->CheckIn(si); fq->Add(si);
  si->ImportKings();
  SpawnEye(si, name, title);
}

/**************************************************************************/
/**************************************************************************/
// Info stream functions
/**************************************************************************/
/**************************************************************************/

void InfoStream(InfoStream_e type, const char* s)
{
  switch (type) {
  case ISoutput:  Gled::theOne->output(s);  break;
  case ISmessage: Gled::theOne->message(s); break;
  case ISwarning: Gled::theOne->warning(s); break;
  case ISerror:   Gled::theOne->error(s);   break;
  }
}

void InfoStream(InfoStream_e type, const string& s)
{
  InfoStream(type, s.c_str());
}

/**************************************************************************/
/**************************************************************************/
