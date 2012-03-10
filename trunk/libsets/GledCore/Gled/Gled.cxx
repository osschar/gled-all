// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Gled.h"
#include "gled-config-build.h"

#include <Gled/GledNS.h>
#include <Gled/GMutex.h>
#include <Gled/GCondition.h>
#include <Gled/GThread.h>
#include <Gled/TRootXTReq.h>
#include <Gled/GKeyRSA.h>
#include <Ephra/Saturn.h>
#include <Glasses/ZQueen.h>
#include <Glasses/SaturnInfo.h>
#include <Glasses/ShellInfo.h>

#include <TSystem.h>
#include <TSysEvtHandler.h>
#include <TROOT.h>
#include <TInterpreter.h>
#include <TRint.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TException.h>
#include <TMath.h>
#include <TPRegexp.h>
#include <TSocket.h>
#include <TObjectTable.h>
#include <TSystemDirectory.h>
#include <TThread.h>
#include <TH1.h>
#include <Getline.h>

extern void *GledCore_GLED_init;

ClassImp(Gled);

/**************************************************************************/

Gled* Gled::theOne      = 0;
int   Gled::sExitStatus = 0;

int Gled::GetExitStatus()
{
  return sExitStatus;
}

/**************************************************************************/

void Gled::next_arg_or_die(lStr_t& args, lStr_i& i)
{
  lStr_i j = i;
  if (++j == args.end() || (*j)[0] == '-' || j->EndsWith(".C"))
  {
    cerr <<"Error: option "<< *i <<" requires an argument.\n";
    exit(1);
  }
  i = j;
}

/**************************************************************************/

Gled::Gled() :
  mSaturn       (0),
  bIsSun        (false),
  bQuit         (false),
  bHasPrompt    (true),
  bShowSplash   (true),
  bPreExec      (false),
  bAutoSpawn    (false),
  bAllowMoons   (false),
  bRootAppRunning(false),
  mRootApp      (0),
  mLoggingMutex (GMutex::recursive),
  mLogFile      (0),
  mOutFile      (0),
  mExitCondVar  (0),
  mRootAppThread(0),
  mExitThread   (0)
{
  if (theOne)
  {
    fprintf(stderr, "Gled::Gled trying to instantiate another Gled object.\n");
    exit(1);
  }

  theOne = this;

  // Set-up SaturnInfo, set defaults

  mSaturnInfo = new SaturnInfo;
  mSaturnInfo->SetHostName(gSystem->HostName());

  SysInfo_t sys; gSystem->GetSysInfo(&sys);
  MemInfo_t mem; gSystem->GetMemInfo(&mem);

  mSaturnInfo->SetOS        (sys.fOS);
  mSaturnInfo->SetCPU_Model (sys.fModel);
  mSaturnInfo->SetCPU_Type  (sys.fCpuType);
  mSaturnInfo->SetCPU_Freq  (sys.fCpuSpeed);
  mSaturnInfo->SetCPU_Num   (sys.fCpus);
  mSaturnInfo->SetMemory    (mem.fMemTotal);
  mSaturnInfo->SetSwap      (mem.fSwapTotal);

  mLogFileName = "<stdout>"; mLogFile = 0;
  mOutFileName = "<stdout>"; mOutFile = 0;

  // Figure out libdir and datadir, can also be passed as arguments later.
  TString gsys(gSystem->Getenv("GLEDSYS"));
  if (gsys.IsNull())
  {
#ifdef GLED_PREFIX
    mLibDir  = GLED_LIB_DIR;
    mDataDir = GLED_DATA_DIR;
#endif
  }
  else
  {
    mLibDir  = gsys + "/lib";
    mDataDir = gsys;
  }
  mHomeDir = gSystem->HomeDirectory();

  mAuthDir = GForm("%s/.gled/auth", mHomeDir.Data());
  mDefEyeIdentity = "guest";
}

void Gled::AddArgument(const char* arg)
{
  // Add a command-line argument.

  mArgs.push_back(arg);
}

void Gled::ReadArguments(int argc, char **argv)
{
  // Read given command-line arguments, zeroth argument is taken as
  // command name.

  mCmdName = argv[0];
  for (int i = 1; i < argc; ++i)
  {
    mArgs.push_back(argv[i]);
  }
}

void Gled::ParseArguments()
{
  // Parse command-line arguments.

  lStr_i i  = mArgs.begin();
  while (i != mArgs.end())
  {
    lStr_i start = i;

    if(*i == "-h" || *i == "-help" || *i == "--help" || *i == "-?")
    {
      printf(
             "Arguments: [options] [dir] [file(s)]\n"
             "                     dir   ~ cd to dir prior to exec of files\n"
             "                     files ~ ROOT macro scripts to process\n"
             "Gled options:\n"
             "-------------\n"
             "  -datadir   <dir>   directory containing Gled data\n"
             "  -libdir    <dir>   directory containing Gled libraries\n"
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
             "  -noprompt          no ROOT prompt (runs TApplication insted of TRint)\n"
             "  -l                 no splash info\n"
             "\n"
             "Logging options:\n"
             "  -log[file] <file>  specify log file name (saturn:'<stdout>', gled:'<null>')\n"
             "  -out[file] <file>  specify output file name (def: '<stdout>')\n"
             "                     <file> shorthands: '-' => '<null>', '+' => '<stdout>'\n"
	     "  -debug     <lvl>   set debug level (some messages require debug build)\n"
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
    else if (*i == "-datadir")
    {
      next_arg_or_die(mArgs, i);
      mDataDir = *i;
      mArgs.erase(start, ++i);
    }
    else if (*i == "-libdir")
    {
      next_arg_or_die(mArgs, i);
      mLibDir = *i;
      mArgs.erase(start, ++i);
    }
    else if (*i == "-preexec")
    {
      next_arg_or_die(mArgs, i);
      bPreExec = true;
      mPreExecString   = *i;
      mArgs.erase(start, ++i);
    }
    else if (*i ==  "-r" || *i == "-run")
    {
      bAutoSpawn = true;
      mArgs.erase(start, ++i);
    }
    else if (*i == "-allowmoons")
    {
      bAllowMoons = true;
      mArgs.erase(start, ++i);
    }
    else if (*i == "-s" || *i == "-sssize")
    {
      next_arg_or_die(mArgs, i);
      if(index(*i, 'e')) {
	int m, e, num = sscanf(*i, "%de%d", &m, &e);
	if(num != 2) { cerr <<"-sssize poor exp format: "<< *i <<endl; exit(1); }
	mSaturnInfo->SetSunSpaceSize( ID_t(TMath::Power(m, e)) );
      } else {
	mSaturnInfo->SetSunSpaceSize( ID_t(atoll(*i)) );
      }
      mArgs.erase(start, ++i);
    }
    else if (*i == "-p" || *i == "-port")
    {
      next_arg_or_die(mArgs, i);
      mSaturnInfo->SetServerPort( atoi(*i) );
      mArgs.erase(start, ++i);
    }
    else if (*i == "-portscan")
    {
      next_arg_or_die(mArgs, i);
      mSaturnInfo->SetServPortScan( atoi(*i) );
      mArgs.erase(start, ++i);
    }
    else if (*i == "-m" || *i == "-master")
    {
      next_arg_or_die(mArgs, i);
      // !!! Cast required by gcc-4.4.1-1ubuntu3, Aug 2009. Seems strange.
      char* col = (char*) strchr(*i, ':');
      if(col) {
	*(col++) = 0;
	UShort_t p = UShort_t(atoi(col));
	mSaturnInfo->SetMasterPort(p);
      }
      mSaturnInfo->SetMasterName(*i);
      mArgs.erase(start, ++i);
    }
    else if (*i == "-n" || *i == "-name")
    {
      next_arg_or_die(mArgs, i);
      mSaturnInfo->SetName(*i);
      mArgs.erase(start, ++i);
    }
    else if (*i == "-t" || *i == "-title")
    {
      next_arg_or_die(mArgs, i);
      mSaturnInfo->SetTitle(*i);
      mArgs.erase(start, ++i);
    }
    else if (*i == "-noprompt")
    {
      bHasPrompt = false;
      mArgs.erase(start, ++i);
    }
    else if (*i == "-l")
    {
      bShowSplash = false;
      mArgs.erase(start, ++i);
    }

    // Logging options

    else if (*i == "-log" || *i == "-logfile")
    {
      next_arg_or_die(mArgs, i);
      if (*i == "-" || *i == "<null>")
      {
	mLogFileName = "<null>";
      }
      else if (*i == "+" || *i == "<stdout>")
      {
	mLogFileName = "<stdout>";
      }
      else
      {
	mLogFileName = *i;
      }
      mArgs.erase(start, ++i);
    }
    else if (*i == "-out" || *i == "-outfile")
    {
      next_arg_or_die(mArgs, i);
      if (*i == "-" || *i == "<null>")
      {
	mOutFileName = "<null>";
      }
      else if (*i == "+" || *i == "<stdout>")
      {
	mOutFileName = "<stdout>";
      }
      else
      {
	mOutFileName = *i;
      }
      mArgs.erase(start, ++i);
    }
    else if (*i == "-debug")
    {
      next_arg_or_die(mArgs, i);
      G_DEBUG = atoi(*i);
      mArgs.erase(start, ++i);
    }

    // Authentication options

    else if (*i == "-auth")
    {
      mSaturnInfo->SetUseAuth(true);
      mArgs.erase(start, ++i);
    }
    else if (*i == "-authdir")
    {
      next_arg_or_die(mArgs, i);
      mAuthDir = *i;
      mArgs.erase(start, ++i);
    }
    else if (*i == "-saturnid")
    {
      next_arg_or_die(mArgs, i);
      mSaturnInfo->SetLogin(*i);;
      mArgs.erase(start, ++i);
    }
    else if (*i == "-eyeid")
    {
      next_arg_or_die(mArgs, i);
      mDefEyeIdentity = *i;
      mArgs.erase(start, ++i);
    }

    // Renderer loading options

    else if (*i == "-rnr")
    {
      next_arg_or_die(mArgs, i);
      mRenderers = *i;
      mArgs.erase(start, ++i);
    }
    else
    {
      ++i;
    }
  }

  if (mLibDir.IsNull() || mDataDir.IsNull())
  {
    cerr << "libdir or datadir unknown.\n";
    exit(1);
  }
}

void Gled::InitLogging()
{
  if (mLogFileName == "<null>")
    mLogFile = 0;
  else if (mLogFileName = "<stdout>")
    mLogFile = stdout;
  else
  {
    mLogFile = fopen(mLogFileName, "w");
    if (mLogFile == 0) {
      perror("Gled::InitLogging opening of log file failed");
      exit(1);
    }
  }

  if (mOutFileName =="<null>")
    mOutFile = 0;
  else if (mOutFileName == "<stdout>")
    mOutFile = stdout;
  else if (mOutFileName == mLogFileName)
    mOutFile = mLogFile;
  else
  {
    mOutFile = fopen(mOutFileName, "w");
    if (mLogFile == 0) {
      perror("Gled::InitLogging opening of output file failed");
      exit(1);
    }
  }

  if(bShowSplash) {
    GMutexHolder mh(mLoggingMutex);
    message("+----------------------------------------------------------+");
    message(GForm("| This is Gled, version %s", GLED_BUILD_VERSION));
    message(GForm("|   Build date: %s", GLED_BUILD_DATE));
    message(GForm("|      SVN Rev: %s", GLED_BUILD_SVN_REV));
    message(GForm("|      SVN URL: %s", GLED_BUILD_SVN_URL));
    message("| Gled is free software, released under GNU GPL version 2");
    message("| For further information visit http://www.gled.org/");
    message("+----------------------------------------------------------+");
    message("Gled now bootstraping.");
  }
}

void Gled::InitGledCore()
{
  gROOT->SetMacroPath(GForm(".:%s/.gled:%s/macros", mHomeDir.Data(), mDataDir.Data()));
  gInterpreter->AddIncludePath(GForm("%s/.gled",  mHomeDir.Data()));
  gInterpreter->AddIncludePath(GForm("%s/macros", mDataDir.Data()));
  gInterpreter->SetProcessLineLock(false);

  GledNS::GledRoot = new TDirectory("Gled", "Gled root directory");
  GledNS::InitFD(0, GledNS::GledRoot);

  TThread a_root_thread; // Enforce ROOT thread init.

  ((void(*)())GledCore_GLED_init)();

  if (mRenderers != "")
  {
    lStr_t rnrs;
    GledNS::split_string(mRenderers.Data(), rnrs, ':');
    for (lStr_i r = rnrs.begin(); r != rnrs.end(); ++r)
      GledNS::AddRenderer(*r);

    GledNS::AssertRenderers();
  }
}

void Gled::ProcessCmdLineMacros()
{
  // Prepare remaining args for ROOT, weed out remaining options

  static const Exc_t _eh("Gled::ProcessCmdLineMacros ");

  // Argument count and strings to be passed to root.
  int         rargc = 0;
  const char* rargv[mArgs.size() + 2];

  rargv[rargc++] = mCmdName;
  if (!bShowSplash)
    rargv[rargc++] = "-l";
  for (lStr_i i = mArgs.begin(); i != mArgs.end(); ++i)
  {
    if ((*i)[0] == '-')
    {
      cerr << "Error: unknown command-line option '" << *i << "'.\n";
      exit(1);
    } else {
      rargv[rargc++] = i->Data();
    }
  }

  // Call pre-exec macros if any.
  if (bPreExec)
  {
    PreExec();
  }

  // Spawn Root Application
  if (bShowSplash)
  {
    printf("Staring ROOT command-line interpreter ...\n");
  }

  if (bHasPrompt)
  {
    TRint *rint = new TRint("TRint", &rargc, (char**) rargv);
    rint->SetPrompt(mCmdName + "[%d] ");
    mRootApp = rint;
  }
  else
  {
    mRootApp = new TApplication("TApplication", &rargc, (char**) rargv);
  }

  // Spawn saturn
  if (bAutoSpawn)
  {
    SpawnSunOrSaturn();
  }

  volatile Bool_t needGetlinemInit = kFALSE;
  try
  {
    GThread::OwnerChanger _chown(mSaturnInfo);

    // This is a shameless cut-n-paste from TRint::Run()

    Long_t retval = 0;
    Int_t  error = 0;

    if (strlen(mRootApp->WorkingDirectory())) {
      // if directory specified as argument make it the working directory
      gSystem->ChangeDirectory(mRootApp->WorkingDirectory());
      TSystemDirectory *workdir = new TSystemDirectory("workdir", gSystem->WorkingDirectory());
      TObject *w = gROOT->GetListOfBrowsables()->FindObject("workdir");
      TObjLink *lnk = gROOT->GetListOfBrowsables()->FirstLink();
      while (lnk) {
        if (lnk->GetObject() == w) {
          lnk->SetObject(workdir);
          lnk->SetOption(gSystem->WorkingDirectory());
          break;
        }
        lnk = lnk->Next();
      }
      delete w;
    }

    // Process shell command line input files
    if (mRootApp->InputFiles()) {
      // Make sure that calls into the event loop
      // ignore end-of-file on the terminal.
      // fInputHandler->DeActivate();
      TIter next(mRootApp->InputFiles());
      RETRY {
        retval = 0; error = 0;
        Int_t nfile = 0;
        TObjString *file;
        while ((file = (TObjString *)next())) {
          char cmd[kMAXPATHLEN+50];
          Bool_t rootfile = kFALSE;
            
          if (file->String().EndsWith(".root") || file->String().BeginsWith("file:")) {
            rootfile = kTRUE;
          } else {
            FILE *mayberootfile = fopen(file->String(),"rb");
            if (mayberootfile) {
              char header[5];
              if (fgets(header,5,mayberootfile)) {
                rootfile = strncmp(header,"root",4)==0;
              }
              fclose(mayberootfile);
            }
          }
          if (rootfile) {
            // special trick to be able to open files using UNC path names
            if (file->String().BeginsWith("\\\\"))
              file->String().Prepend("\\\\");
            file->String().ReplaceAll("\\","/");
            const char *rfile = (const char*)file->String();
            Printf("Attaching file %s as _file%d...", rfile, nfile);
            snprintf(cmd, kMAXPATHLEN+50, "TFile *_file%d = TFile::Open(\"%s\")", nfile++, rfile);
          } else {
            Printf("Processing %s...", (const char*)file->String());
            snprintf(cmd, kMAXPATHLEN+50, ".x %s", (const char*)file->String());
          }
          if (bHasPrompt) {
            Getlinem(kCleanUp, 0);
            Gl_histadd(cmd);
          }

          // The ProcessLine might throw an 'exception'.  In this case,
          // GetLinem(kInit,"Root >") is called and we are jump back
          // to RETRY ... and we have to avoid the Getlinem(kInit, GetPrompt());
          needGetlinemInit = kFALSE;
          retval = mRootApp->ProcessLine(cmd, kFALSE, &error);
          gCint->EndOfLineAction();
	  if (retval) {
	    Printf("Command '%s' returned %ld.", cmd, retval);
	  }

          // The ProcessLine has successfully completed and we need
          // to call Getlinem(kInit, GetPrompt());
          needGetlinemInit = kTRUE;

          if (error != 0) break;
        }
      } ENDTRY;
    }
  }
  catch(exception& exc)
  {
    fprintf(stderr, "%sexception caught during macro processing:\n%s\n",
            _eh.Data(), exc.what());
    exit(1);
  }

  if (mRootApp->InputFiles()) {
    mRootApp->ClearInputFiles();
  } else {
    if (bHasPrompt && needGetlinemInit)
      Getlinem(kCleanUp, 0);
  }
  if (bHasPrompt && needGetlinemInit) {
    Getlinem(kInit, ((TRint*)mRootApp)->GetPrompt());
  }

  if (bAllowMoons)
  {
    AllowMoonConnections();
  }
}

void Gled::StopLogging()
{
  message("Logging stopped.");

  GMutexHolder mh(mLoggingMutex);

  if (mOutFile && mOutFile != stdout) {
    fclose(mOutFile);
  }
  if (mLogFile && mLogFile != stdout && mLogFile != mOutFile) {
    fclose(mLogFile);
  }
  mOutFile = mLogFile = 0;
}

Gled::~Gled()
{
  delete mSaturn;
  delete mRootApp;
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

void Gled::SpawnSunOrSaturn()
{
  if (mSaturnInfo->RefMasterName().IsNull())
  {
    SpawnSun();
  }
  else
  {
    SpawnSaturn();
  }
}

void Gled::SpawnSun()
{
  static const Exc_t _eh("Gled::SpawnSun ");

  if (mSaturn) return;
  mSaturnInfo->SetMasterPort(0);

  if (strcmp(mSaturnInfo->GetName(), "SaturnInfo") == 0)
  {
    mSaturnInfo->SetName(GForm("Sun at %s", gSystem->HostName()));
  }
  if (strcmp(mSaturnInfo->GetLogin(), "") == 0)
  {
    mSaturnInfo->SetLogin("sun.absolute");
  }

  CheckAuthDir();

  if (mSaturnInfo->GetUseAuth())
  {
    GKeyRSA::init_ssl();
    if (GetPrivKeyFile(mSaturnInfo->mLogin) == 0)
    {
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

  if (mSaturn) return;
  if (strcmp(mSaturnInfo->GetName(), "SaturnInfo") == 0)
  {
    mSaturnInfo->SetName(GForm("Saturn at %s", gSystem->HostName()));
  }
  if (strcmp(mSaturnInfo->GetLogin(), "") == 0)
  {
    mSaturnInfo->SetLogin("saturn");
  }

  CheckAuthDir();

  // Initialize authentication
  GKeyRSA::init_ssl();
  // Warn for missing RSA-key files
  if (GetPrivKeyFile(mSaturnInfo->mLogin, false) == 0)
    ISwarn(_eh + "private key for Saturn identity not found");
  if (GetPrivKeyFile(mDefEyeIdentity, false) == 0)
    ISwarn(_eh + "private key for default Eye identity not found");

  mSaturn = new Saturn;
  try
  {
    SaturnInfo* si = mSaturn->Connect(mSaturnInfo);
    if (si)
    {
      delete mSaturnInfo;
      mSaturnInfo = si;
    }
    else
    {
      cerr << _eh <<"failed ... dying\n";
      exit(1);
    }
    WaitUntillQueensLoaded();
  }
  catch(Exc_t& exc)
  {
    cerr << _eh <<" failed ... dying at:\n  "<< exc <<endl;
    exit(1);
  }
}

/**************************************************************************/

void Gled::CheckAuthDir()
{
  if(gSystem->AccessPathName(mAuthDir.Data(), kReadPermission)) {
    printf("Gled::CheckAuthDir auth dir '%s' not accessible\n", mAuthDir.Data());
    printf("Gled::CheckAuthDir use gled-auth-init command to create one\n");
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

void Gled::Exit(Int_t status)
{
  sExitStatus = status;
  static GMutex exit_moo;
  {
    GMutexHolder lck(exit_moo);
    if (mExitThread)
      return;
    mExitThread = new GThread("Gled Terminator", Exit_tl, 0, true);
  }
  mExitThread->Spawn();
}

void* Gled::Exit_tl(void*)
{
  theOne->ExitVirtual();
  GledNS::ShutdownLibSets();
  // Shutdown GledCore manually, when needed.
  return 0;
}

void Gled::ExitVirtual()
{
  if (mSaturn)
  {
    mSaturn->Shutdown();
  }
  bQuit = true;
  if (mExitCondVar)
  {
    mExitCondVar->Lock();
    mExitCondVar->Signal();
    mExitCondVar->Unlock();
  }
}

/**************************************************************************/

Int_t Gled::LoadLibSet(const Text_t* lib_set)
{
  if (lib_set == 0 || strlen(lib_set) == 0)
  {
    warning("Gled::LoadLibSet() called with an empty argument. Ignoring.");
    return 9;
  }
  return GledNS::LoadSoSet(lib_set);
}

Int_t Gled::LoadLibSet(LID_t lid)
{
  if(GledNS::IsLoaded(lid)) {
    return 0;
  } else {
    Text_t buf[80];
    FILE* f = fopen(GForm("%s/gled_lid_%u", mLibDir.Data(), lid), "r");
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

void Gled::SetDebugLevel(Int_t d)
{
  if(d<0) d=0;
  G_DEBUG = d;
}

/**************************************************************************/
// Info Stream methods
/**************************************************************************/

void Gled::output(const char* s)
{
  if(mOutFile) {
    GThread::CancelDisabler tka;
    GMutexHolder     mh(mLoggingMutex);
    if(mOutFile == mLogFile)
      fputs("OUT: ", mOutFile);
    fputs(s, mOutFile);
    putc(10, mOutFile);
  }
}

void Gled::message(const char* s)
{
  if(mLogFile) {
    GThread::CancelDisabler tka;
    GMutexHolder     mh(mLoggingMutex);
    fputs("MSG: ", mLogFile);
    fputs(s, mLogFile);
    putc(10, mLogFile);
  }
}

void Gled::warning(const char* s)
{
  if(mLogFile) {
    GThread::CancelDisabler tka;
    GMutexHolder     mh(mLoggingMutex);
    fputs("WRN: ", mLogFile);
    fputs(s, mLogFile);
    putc(10, mLogFile);
  }
}

void Gled::error(const char* s)
{
  if(mLogFile) {
    GThread::CancelDisabler tka;
    GMutexHolder     mh(mLoggingMutex);
    fputs("ERR: ", mLogFile);
    fputs(s, mLogFile);
    putc(10, mLogFile);
  }
}

/**************************************************************************/

EyeInfo* Gled::SpawnEye(const TString& libset, const TString& eyector)
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

namespace
{
  struct MacroEls
  {
    TString fFoo, fExt, fArgs;

    MacroEls(TPMERegexp& r) : fFoo(r[1]), fExt(r[2]), fArgs(r[3]) {}

    TString FunctionCall() const { return fFoo + "(" + fArgs + ")"; }
    TString MacroFile()    const { return fFoo + fExt; }
  };

  MacroEls parse_macro(const TString& s)
  {
    TPMERegexp macro_re("(\\w+)(\\.\\w+)(?:\\((.*)\\))?");

    Int_t n = macro_re.Match(s);
    if (n != 3 && n != 4)
    {
      cout << "A fishy macro request received: '" << s << "'. There's a bumpy road ahead.\n";
    }
    if (n >= 3 && macro_re[2] != ".C")
    {
      cout << "A macro without '.C' received: '" << s << "'. Not sure we'll get much further.\n";
    }

    return MacroEls(macro_re);
  }
}

void Gled::AssertMacro(const Text_t* mac)
{
  // Load and execute macro 'mac' if it has not been loaded yet.

  MacroEls m = parse_macro(mac);
  if (gROOT->GetGlobalFunction(m.fFoo, 0, true) == 0)
  {
    gROOT->LoadMacro(m.MacroFile());
    gROOT->ProcessLine(m.FunctionCall());
  }
}

void Gled::Macro(const Text_t* mac)
{
  // Execute macro 'mac'. Do not reload the macro.

  MacroEls m = parse_macro(mac);
  if (gROOT->GetGlobalFunction(m.fFoo, 0, true) == 0)
  {
    gROOT->LoadMacro(m.MacroFile());
  }
  gROOT->ProcessLine(m.FunctionCall());
}

void Gled::LoadMacro(const Text_t* mac)
{
  // Makes sure that macro 'mac' is loaded, but do not reload it.

  MacroEls m = parse_macro(mac);
  if (gROOT->GetGlobalFunction(m.fFoo, 0, true) == 0)
  {
    gROOT->LoadMacro(m.MacroFile());
  }
}

/**************************************************************************/
// Thread foos
/**************************************************************************/

namespace
{
  class GExceptionHandler : public TStdExceptionHandler
  {
  public:
    GExceptionHandler() : TStdExceptionHandler() { Add(); }
    virtual ~GExceptionHandler()                 { Remove(); }

    virtual EStatus  Handle(std::exception& exc)
    {
      // Handle exceptions deriving from TEveException.

      Exc_t* ex = dynamic_cast<Exc_t*>(&exc);
      if (ex) {
        cout <<"RootApp runner caught exception: "<< ex << endl;
        cout <<"RootApp runner reentering event loop ...\n";
        return kSEHandled;
      } else {
        cout <<"RootApp runner caught std exception: "<< exc.what() <<endl;
        return kSEProceed;
      }
    }
  };

  class GTerminateHandler : public TSignalHandler
  {
  public:
    GTerminateHandler() : TSignalHandler(kSigTermination, kTRUE) { Add(); }
    virtual ~GTerminateHandler() {}

    virtual Bool_t Notify()
    {
      cout << "Received SIGTERM ... initiating shutdown.\n";
      gSystem->ExitLoop();
      return kFALSE;
    }
  };

  // This one is only use when no prompt / TApplication is used.
  class GInterruptHandler : public TSignalHandler
  {
  public:
    GInterruptHandler() : TSignalHandler(kSigInterrupt, kTRUE) { Add(); }
    virtual ~GInterruptHandler() {}

    virtual Bool_t Notify()
    {
      cout << "\nReceived SIGINT ... initiating shutdown.\n";
      gSystem->ExitLoop();
      return kFALSE;
    }
  };

  class GSigChildHandler : public TSignalHandler
  {
  public:
    GSigChildHandler() : TSignalHandler(kSigChild, kTRUE) { Add(); }
    virtual ~GSigChildHandler() {}

    virtual Bool_t Notify() { return kTRUE; }
  };
}

GThread* Gled::SpawnRootAppThread(const TString& name_prefix)
{
  // Spawns thread running ROOT's main event loop.
  // Makes sure the stack size is at least 8MB.

  static const Exc_t _eh("Gled::SpawnRootAppThread ");

  GThread* thr = new GThread(name_prefix + "-TRintRunner",
                             (GThread_foo) RootApp_runner_tl, 0, false);
  thr->SetStackSize(8*1024*1024);
  if (thr->Spawn())
  {
    perror(_eh + "can not create RootApp thread, aborting.");
    exit(1);
    return 0;
  }
  return thr;
}

void* Gled::RootApp_runner_tl(void*)
{
  // Runs the ROOT application.
  // Ownership set to mSaturnInfo.

  GThread* self = GThread::Self();

  self->set_owner(Gled::theOne->mSaturnInfo);

  Gled::theOne->mRootAppThread = self;

  // Activate some signals -- this thread handles most system signals.
  // Handler for those (and for CPU exceptions) is installed in
  // TUnixSystem::Init(). This is called from TROOT constructor.
  // And TROOT constructor is called via static initialization of gROOT.
  //
  // For TRint, SigINT is set from TRint constructor (instantiated in, not
  // very intuitively, in ProcessCmdLineMacros()). For TApplication we install
  // our own ... that will exit like SigTERM.

  GThread::UnblockSignal(GThread::SigHUP);
  GThread::UnblockSignal(GThread::SigINT);
  GThread::UnblockSignal(GThread::SigTERM);
  GThread::UnblockSignal(GThread::SigCONT);
  GThread::UnblockSignal(GThread::SigTSTP);
  GThread::UnblockSignal(GThread::SigPIPE);
  GThread::UnblockSignal(GThread::SigCHLD);
  GThread::UnblockSignal(GThread::SigALRM);
  GThread::UnblockSignal(GThread::SigURG);
  GThread::UnblockSignal(GThread::SigSYS);
  GThread::UnblockSignal(GThread::SigWINCH);

  GThread::SetDefaultSignalHandler(GThread::ToRootsSignalHandler);
  self->SetTerminalPolicy(GThread::TP_GledExit);

  // Root does not want TThread to exist for the main thread.
  self->ClearRootTThreadRepresentation();

  // The sig-handlers will be deleted in ~TROOT().
  new GSigChildHandler;
  new GTerminateHandler;
  if ( ! Gled::theOne->GetHasPrompt())
  {
    new GInterruptHandler;
  }

  new GExceptionHandler;

  // Global ROOT settings.
  TH1::AddDirectory(kFALSE);

  self->CleanupPush((GThread_cu_foo) RootApp_cleanup_tl, 0);

  Gled::theOne->ProcessCmdLineMacros();

  TRootXTReq::Bootstrap(self);
  GThread::UnblockSignal(GThread::SigUSR1);

  Gled::theOne->bRootAppRunning = true;
  Gled::theOne->mRootApp->TApplication::Run(true);
  Gled::theOne->bRootAppRunning = false;
  cout << "TApplication::Run() exit ...\n";

  GThread::BlockSignal(GThread::SigUSR1);
  TRootXTReq::Shutdown();

  self->CleanupPop(false);

  if (Gled::theOne->GetQuit() == false)
    Gled::theOne->Exit();

  Gled::theOne->mRootAppThread = 0;

  return 0;
}

void Gled::RootApp_cleanup_tl(void*)
{
  cout << "Thread running TApplication::Run() canceled ... expect trouble.\n";
  Gled::theOne->mRootApp->Terminate(0);
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
