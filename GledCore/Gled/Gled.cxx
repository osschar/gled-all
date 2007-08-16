// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Gled.h"
#include <Gled/GledNS.h>
#include <Gled/GMutex.h>
#include <Gled/GCondition.h>
#include <Gled/GThread.h>
#include <Gled/GKeyRSA.h>
#include <Ephra/Saturn.h>
#include <Glasses/ZQueen.h>
#include <Glasses/SaturnInfo.h>
#include <Glasses/ShellInfo.h>

#include <TSystem.h>
#include <TROOT.h>
#include <TRint.h>
#include <TMath.h>
#include <TSocket.h>
#include <TObjectTable.h>
#include <TSystemDirectory.h>

extern void *GledCore_GLED_init;

ClassImp(Gled)

/**************************************************************************/

Gled* Gled::theOne = 0;
Gled* gGled        = 0;

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

Gled::Gled() :
  mSaturn       (0),
  bIsSun        (false),
  bQuit         (false),
  bShowSplash   (true),
  bPreExec      (false),
  bAutoSpawn    (false),
  bAllowMoons   (false),
  bRunRint      (true),
  bRintRunning  (false),
  mLoggingMutex (GMutex::recursive),
  mLogFile      (0),
  mOutFile      (0),
  mExitCondVar  (0)
{
  if(theOne) {
    cerr <<"Gled::Gled trying to instantiate another object ...\n";
    exit(1);
  }
  gGled = theOne = this;
  GThread::init_tsd();

  // Set-up SaturnInfo, set defaults

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

  mLogFileName = "<stdout>"; mLogFile = 0;
  mOutFileName = "<stdout>"; mOutFile = 0;

  mAuthDir = GForm("%s/.gled/auth", gSystem->Getenv("HOME"));
  mDefEyeIdentity = "guest";
}

void Gled::ParseArguments(list<char*>& args)
{
  // Parse command-line arguments.

  list<char*>::iterator i = args.begin();
  while(i != args.end()) {
    list<char*>::iterator start = i;

    if(strcmp(*i, "-h")==0 || strcmp(*i, "-help")==0 ||
       strcmp(*i, "--help")==0 || strcmp(*i, "-?")==0)
      {
	printf(
	       "Arguments: [options] [dir] [file(s)]\n"
	       "                     dir   ~ cd to dir prior to exec of files\n"
	       "                     files ~ ROOT macro scripts to process\n"
	       "Gled options:\n"
	       "-------------\n"
	       "  -preexec <m1:m2..> pre-exec specified macros\n"
	       "  -r[un]             spawn Saturn/Sun immediately (before processing files)\n"
	       "                     Saturn if -master is specified, Sun otherwise\n"
	       "  -allowmoons        accept moon connections\n"
	       "  -s[ssize]  <num>   specify size of sun-space (can be eg. 2e20)\n"
	       "  -p[ort]    <num>   specify server port (def: 9061)\n"
	       "  -portscan  <num>   if server port can not be opened, try <num> higher ports\n"
	       "  -m[aster] <host>[:<port>] master Saturn address (def port: 9061)\n"
	       "  -n[ame]    <str>   name of Saturn\n"
	       "  -t[itle]   <str>   title of Saturn\n"
	       "  -l                 no splash info\n"
	       "  -norint            do not run TRint (useful for batch saturns)\n"
	       "\n"
	       "Logging options:\n"
	       "  -log[file] <file>  specify log file name (saturn:'<stdout>', gled:'<null>')\n" 
	       "  -out[file] <file>  specify output file name (def: '<stdout>')\n"
	       "                     <file> shorthands: '-' => '<null>', '+' => '<stdout>'\n"
	       "\n"
	       "Authentication options:\n"
	       "  -auth              use authentication\n"
	       "  -authdir   <str>   directory containing auth data (def: ~/.gled/auth)\n"
	       "  -saturnid  <str>   identity of the Saturn (def: 'sun.absolute' or 'saturn')\n"
	       "  -eyeid     <str>   default identity of Eyes (def: 'guest')\n"
	       "Renderer loading options:\n"
	       "  -rnr <r1>:<r2>:... specify which rendering libraries to load (for gled: GL)\n"
	       );
	bQuit = true;
	return;
      }

    else if(strcmp(*i, "-preexec")==0) {
      next_arg_or_die(args, i);
      bPreExec = true;
      mPreExecString   = *i;
      args.erase(start, ++i);
    }

    else if(strcmp(*i, "-r")==0 || strcmp(*i, "-run")==0) {
      bAutoSpawn = true;
      args.erase(start, ++i);
    }

    else if(strcmp(*i, "-allowmoons")==0) {
      bAllowMoons = true;
      args.erase(start, ++i);
    }

    else if(strcmp(*i, "-s")==0 || strcmp(*i, "-sssize")==0) {
      next_arg_or_die(args, i);
      if(index(*i, 'e')) {
	int m, e, num = sscanf(*i, "%de%d", &m, &e);
	if(num != 2) { cerr <<"-sssize poor exp format: "<< *i <<endl; exit(1); }
	mSaturnInfo->SetSunSpaceSize( ID_t(TMath::Power(m, e)) );
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

    else if(strcmp(*i, "-portscan")==0) {
      next_arg_or_die(args, i);
      mSaturnInfo->SetServPortScan( atoi(*i) );
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

    // Logging options

    else if(strcmp(*i, "-log")==0 || strcmp(*i, "-logfile")==0) {
      next_arg_or_die(args, i);
      if(strcmp(*i, "-")==0 || strcmp(*i, "<null>")==0) {
	mLogFileName = "<null>";
      } else if(strcmp(*i, "+")==0 || strcmp(*i, "<stdout>")==0) {
	mLogFileName = "<stdout>";
      } else {
	mLogFileName = *i;
      }
      args.erase(start, ++i);
    }

    else if(strcmp(*i, "-out")==0 || strcmp(*i, "-outfile")==0) {
      next_arg_or_die(args, i);
      if(strcmp(*i, "-")==0 || strcmp(*i, "<null>")==0) {
	mOutFileName = "<null>";
      } else if(strcmp(*i, "+")==0 || strcmp(*i, "<stdout>")==0) {
	mOutFileName = "<stdout>";
      } else {
	mOutFileName = *i;
      }
      args.erase(start, ++i);
    }

    // Authentication options

    else if(strcmp(*i, "-auth")==0) {
      mSaturnInfo->SetUseAuth(true);
      args.erase(start, ++i);
    }

    else if(strcmp(*i, "-authdir")==0) {
      next_arg_or_die(args, i);
      mAuthDir = *i;
      args.erase(start, ++i);
    }

    else if(strcmp(*i, "-saturnid")==0) {
      next_arg_or_die(args, i);
      mSaturnInfo->SetLogin(*i);;
      args.erase(start, ++i);
    }

    else if(strcmp(*i, "-eyeid")==0) {
      next_arg_or_die(args, i);
      mDefEyeIdentity = *i;
      args.erase(start, ++i);
    }

    // Renderer loading options

    else if(strcmp(*i, "-rnr")==0) {
      next_arg_or_die(args, i);
      mRenderers = *i;
      args.erase(start, ++i);
    }

    else {
      ++i;
    }

  }

}

void Gled::InitLogging()
{
  if(mLogFileName.CompareTo("<null>") == 0)
    mLogFile = 0;
  else if(mLogFileName.CompareTo("<stdout>") == 0)
    mLogFile = stdout;
  else {
    mLogFile = fopen(mLogFileName.Data(), "w");
    if(mLogFile == 0) {
      perror("Gled::InitLogging opening of log file failed");
      exit(1);
    }
  }

  if(mOutFileName.CompareTo("<null>") == 0)
    mOutFile = 0;
  else if (mOutFileName.CompareTo("<stdout>") == 0)
    mOutFile = stdout;
  else if(mOutFileName.CompareTo(mLogFileName) == 0)
    mOutFile = mLogFile;
  else {
    mOutFile = fopen(mOutFileName.Data(), "w");
    if(mLogFile == 0) {
      perror("Gled::InitLogging opening of output file failed");
      exit(1);
    }
  }
}

void Gled::InitGledCore()
{
  if(bShowSplash) {
    int len = strlen(GLED_VERSION_STRING) + strlen(GLED_BUILD_DATE_STRING) + 4;
    GMutexHolder mh(mLoggingMutex);
    message("+----------------------------------------------------------+");
    message(GForm("| This is Gled, version %s, %s %*s |",
		  GLED_VERSION_STRING, GLED_BUILD_DATE_STRING, 35 - len, "" ));
    message("| Gled is free software, released under GNU GPL version 2. |");
    message("| For further information visit http://www.gled.org/       |");
    message("+----------------------------------------------------------+");
    message("Gled now bootstraping.");
  }

  ((void(*)())GledCore_GLED_init)();

  if(mRenderers != "") {
    lStr_t rnrs;
    GledNS::split_string(mRenderers.Data(), rnrs, ':');
    for(lStr_i r=rnrs.begin(); r!=rnrs.end(); ++r)
      GledNS::AddRenderer(*r);

    GledNS::AssertRenderers();
  }
}

void Gled::StopLogging()
{
  message("Logging stopped.");

  GMutexHolder mh(mLoggingMutex);

  if (mOutFile && mOutFile != stdin) {
    fclose(mOutFile);
  }
  if (mLogFile && mLogFile != stdin && mLogFile != mOutFile) {
    fclose(mLogFile);
  }
  mOutFile = mLogFile = 0;
}

Gled::~Gled() {
  delete mSaturn;
}

/**************************************************************************/

void Gled::PreExec()
{
  lStr_t l;
  GledNS::split_string(mPreExecString, l, ':');
  for(lStr_i i=l.begin(); i!=l.end(); ++i)
    gROOT->Macro(i->Data());
}

/**************************************************************************/

void Gled::SpawnSunOrSaturn() {
  if(mSaturnInfo->RefMasterName().IsNull()) {
    SpawnSun();
  } else {
    SpawnSaturn();
  }
}

void Gled::SpawnSun()
{
  static const Exc_t _eh("Gled::SpawnSun ");

  if(mSaturn) return;
  mSaturnInfo->SetMasterPort(0);

  if(strcmp(mSaturnInfo->GetName(), "SaturnInfo") == 0)
    mSaturnInfo->SetName(GForm("Sun at %s", gSystem->HostName()));
  if(strcmp(mSaturnInfo->GetLogin(), "") == 0)
    mSaturnInfo->SetLogin("sun.absolute");

  CheckAuthDir();

  if(mSaturnInfo->GetUseAuth()) {
    GKeyRSA::init_ssl();
    if(GetPrivKeyFile(mSaturnInfo->mLogin) == 0) {
      cerr << _eh << "can not open server private key\n";
      exit(1);
    }
  }

  mSaturn = new Saturn;
  mSaturn->Create(mSaturnInfo);
  bIsSun = true;
}

void Gled::SpawnSaturn()
{
  static const Exc_t _eh("Gled::SpawnSaturn ");

  if(mSaturn) return;
  if(strcmp(mSaturnInfo->GetName(), "SaturnInfo") == 0)
    mSaturnInfo->SetName(GForm("Saturn at %s", gSystem->HostName()));
  if(strcmp(mSaturnInfo->GetLogin(), "") == 0)
    mSaturnInfo->SetLogin("saturn");

  CheckAuthDir();

  // Initialize authentication
  GKeyRSA::init_ssl();
  // Warn for missing RSA-key files
  if(GetPrivKeyFile(mSaturnInfo->mLogin, false) == 0)
    ISwarn(_eh + "private key for Saturn identity not found");
  if(GetPrivKeyFile(mDefEyeIdentity, false) == 0)
    ISwarn(_eh + "private key for default Eye identity not found");

  mSaturn = new Saturn;
  try {
    SaturnInfo* si = mSaturn->Connect(mSaturnInfo);
    if(si) {
      delete mSaturnInfo;
      mSaturnInfo = si;
    } else {
      cerr << _eh <<"failed ... dying\n";
      exit(1);
    }
    WaitUntillQueensLoaded();
  }
  catch(Exc_t& exc) {
    cerr << _eh <<" failed ... dying at:\n  "<< exc <<endl;
    exit(1);
  }
}

/**************************************************************************/

void Gled::CheckAuthDir() {
  if(gSystem->AccessPathName(mAuthDir.Data(), kReadPermission)) {
    printf(GForm("Gled::SpawnSaturn: auth dir '%s' not accessible\n", mAuthDir.Data()));
    printf("Gled::SpawnSaturn: use gled-auth-init command to create one\n");
  }
}

const char* Gled::PubKeyFile(TString& id)
{
  return GForm("%s/public_keys/%s", mAuthDir.Data(), id.Data());
}

const char* Gled::PrivKeyFile(TString& id)
{
  return GForm("%s/private_keys/%s", mAuthDir.Data(), id.Data());
}

const char* Gled::GetPubKeyFile(TString& id, Bool_t use_exc)
{
  static const Exc_t _eh("Gled::GetPubKeyFile ");

  const char* ret = PubKeyFile(id);
  if(gSystem->AccessPathName(ret, kReadPermission)) {
    if(use_exc)
      throw(_eh + "can not access file:" + ret + ".");
    else
      ret = 0;
  }
  return ret;
}

const char* Gled::GetPrivKeyFile(TString& id, Bool_t use_exc)
{
  static const Exc_t _eh("Gled::GetPrivKeyFile ");

  const char* ret = PrivKeyFile(id);
  if(gSystem->AccessPathName(ret, kReadPermission)) {
    if(use_exc)
      throw(_eh + "can not access file:" + ret + ".");
    else
      ret = 0;
  }
  return ret;
}

/**************************************************************************/

Bool_t Gled::IsIdentityInGroup(const char* id, const char* group)
{
  //printf("Gled::IsIdentityInGroup checking if %s in group %s\n", id, group);
  return (gSystem->Exec(GForm("grep -q %s %s/groups/%s",
			      id, mAuthDir.Data(), group)) == 0) ? true : false;;
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
    FILE* f = fopen(GForm("%s/lib/.%u", gSystem->Getenv("GLEDSYS"), lid), "r");
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

/**************************************************************************/

void Gled::SetDebugLevel(Int_t d) {
  if(d<0) d=0;
  G_DEBUG = d;
}

/**************************************************************************/
// Info Stream methods
/**************************************************************************/

void Gled::output(const char* s) {
  if(mOutFile) {
    GThreadKeepAlive tka;
    GMutexHolder     mh(mLoggingMutex);
    if(mOutFile == mLogFile)
      fputs("OUT: ", mOutFile);
    fputs(s, mOutFile);
    putc(10, mOutFile);
  }
}

void Gled::message(const char* s) {
  GThread::SetCancelState(GThread::CS_Disable);
  if(mLogFile) {
    GThreadKeepAlive tka;
    GMutexHolder     mh(mLoggingMutex);
    fputs("MSG: ", mLogFile);
    fputs(s, mLogFile);
    putc(10, mLogFile);
  }
  GThread::SetCancelState(GThread::CS_Enable);
}

void Gled::warning(const char* s) {
  if(mLogFile) {
    GThreadKeepAlive tka;
    GMutexHolder     mh(mLoggingMutex);
    fputs("WRN: ", mLogFile);
    fputs(s, mLogFile);
    putc(10, mLogFile);
  }
}

void Gled::error(const char* s) {
  if(mLogFile) {
    GThreadKeepAlive tka;
    GMutexHolder     mh(mLoggingMutex);
    fputs("ERR: ", mLogFile);
    fputs(s, mLogFile);
    putc(10, mLogFile);
  }
}

/**************************************************************************/

EyeInfo* Gled::SpawnEye(const char* libset, const char* eyector)
{
  if(mSaturn == 0) {
    ISerr("Gled::SpawnEye Saturn has not been spawned");
    return 0;
  }

  return SpawnEye(0, 0, libset, eyector);
}

/**************************************************************************/

void Gled::AssertLibSet(const Text_t* lib_set)
{
  // Makes sure libset 'lib_set' is loaded.

  if(GledNS::IsLoaded(lib_set) == false)
    theOne->LoadLibSet(lib_set);
}

namespace {
  void chomp_tail(TString& s, char c='.') {
    Ssiz_t p = s.Last(c);
    if(p != kNPOS)
      s.Remove(p);
  }
}

void Gled::AssertMacro(const Text_t* mac)
{
  // Load and execute macro 'mac' if it has not been loaded yet.

  TString foo(mac); chomp_tail(foo);
  if(gROOT->GetGlobalFunction(foo.Data(), 0, true) == 0) {
    gROOT->Macro(mac);
  }
}

void Gled::Macro(const Text_t* mac)
{
  // Execute macro 'mac'. Do not reload the macro.

  TString foo(mac); chomp_tail(foo);
  if(gROOT->GetGlobalFunction(foo.Data(), 0, true) == 0)
    gROOT->LoadMacro(mac);

  foo += "()";
  gROOT->ProcessLine(foo.Data());
}

void Gled::LoadMacro(const Text_t* mac)
{
  // Makes sure that macro 'mac' is loaded, but do not reload it.

  TString foo(mac); chomp_tail(foo);
  if(gROOT->GetGlobalFunction(foo.Data(), 0, true) == 0)
    gROOT->LoadMacro(mac);
}

/**************************************************************************/
// Thread foos
/**************************************************************************/

void* Gled::TRint_runner_tl(TRint* gint)
{
  // Runs the ROOT application. Ownership set to mSaturnInfo.

  GThread::setup_tsd(Gled::theOne->mSaturnInfo);

  Gled::theOne->bRintRunning = true;
  while(Gled::theOne->bRintRunning) {
    // !!! Attempt to catch exceptions from root.
    // !!! Does not work.
    try {
      gint->TApplication::Run(true);
      Gled::theOne->bRintRunning = false;
      cout << "Gint terminated ...\n";
    }
    catch(Exc_t& exc) {
      cout <<"TRint runner caught exception: "<< exc << endl;
      cout <<"TRint runner reentering event loop ...\n";
    }
  }

  if(Gled::theOne->GetQuit()==false) Gled::theOne->Exit();
  GThread::Exit();
  return 0;
}

void* Gled::Gled_runner_tl(Gled* gled)
{
  // Runs Gled UI. Only spawned from gled.cxx.

  GThread::setup_tsd(Gled::theOne->mSaturnInfo);

  gled->Run();
  GThread::Exit();
  return 0;
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

void InfoStream(InfoStream_e type, const TString& s)
{
  InfoStream(type, s.Data());
}

void InfoStream(InfoStream_e type, const string& s)
{
  InfoStream(type, s.c_str());
}

/**************************************************************************/
/**************************************************************************/
