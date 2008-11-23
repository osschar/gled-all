// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Gled_H
#define GledCore_Gled_H

// Includes
#include <Gled/GledTypes.h>
#include <Gled/GMutex.h>

class Saturn;
class SaturnInfo;
class EyeInfo;
class ShellInfo;
class Forest;
class Mountain;
class Eye;

class GCondition;
class GThread;

class Fl_SWM_Manager;
class Fl_Window;
class Fl_Tile;
class Fl_Browser;

class TRint;
class TCanvas;

class Gled
{
protected:
  void next_arg_or_die(lStr_t& args, lStr_i& i);

  SaturnInfo*	mSaturnInfo;	// X{g}
  Saturn*	mSaturn;	// X{g}
  Bool_t	bIsSun;		// X{G}

  Bool_t	bQuit;		// X{G}
  Bool_t	bShowSplash;	// X{G}
  Bool_t	bPreExec;       // X{G}
  TString	mPreExecString; // X{G}
  Bool_t	bAutoSpawn;	// X{G}
  Bool_t	bAllowMoons;	// X{G}

  Bool_t	bRunRint;	// X{G}
  Bool_t	bRintRunning;	// X{G}
  TRint        *mRint;          // X{g}

  GMutex	mLoggingMutex;
  TString	mLogFileName;
  FILE*		mLogFile;
  TString	mOutFileName;
  FILE*		mOutFile;

  TString	mAuthDir;	 // X{GS}
  TString	mDefEyeIdentity; // X{GS}

  TString	mRenderers;      // X{GS}

  GCondition*	mExitCondVar;
  GThread*      mRintThread;     // X{g}

public:
  static  void InitStatics();

  Gled();
  virtual void ParseArguments(lStr_t& args);
  virtual void InitLogging();
  virtual void InitGledCore();
  virtual void ProcessCmdLineMacros(const TString& exe, lStr_t& args);
  virtual void StopLogging();
  virtual ~Gled();

  void SetExitCondVar(GCondition* cond) { mExitCondVar = cond; }

  void PreExec();

  void SpawnSunOrSaturn();
  void SpawnSun();
  void SpawnSaturn();

  void        CheckAuthDir();
  const char* PubKeyFile(TString& id);
  const char* PrivKeyFile(TString& id);
  const char* GetPubKeyFile(TString& id, Bool_t use_exc=true);
  const char* GetPrivKeyFile(TString& id, Bool_t use_exc=true);

  Bool_t IsIdentityInGroup(const char* id, const char* group);

  void WaitUntillQueensLoaded();
  void AllowMoonConnections();

  virtual void Run() {}
  virtual void Exit();

  virtual Int_t LoadLibSet(const Text_t* lib_set);
  virtual Int_t LoadLibSet(LID_t lid);

  virtual void SetDebugLevel(Int_t d=0);

  virtual void output(const char* s);
  virtual void message(const char* s);
  virtual void warning(const char* s);
  virtual void error(const char* s);

  virtual Bool_t HasGUILibs() const { return false; }

  virtual EyeInfo* SpawnEye(const char* libset, const char* eyector);
  virtual EyeInfo* SpawnEye(EyeInfo* ei, ZGlass* ud,
			    const char* libset, const char* eyector)
  { return 0; }

  virtual TCanvas*
  NewCanvas(const Text_t* name="Canvas", const Text_t* title=0,
	    int w=640, int h=480, int npx=1, int npy=1)
  { return 0; }

  static void AssertLibSet(const Text_t* lib_set);

  static void AssertMacro(const Text_t* mac);
  static void Macro(const Text_t* mac);
  static void LoadMacro(const Text_t* mac);

  static void* TRint_runner_tl(TRint* gint);
  static void  TRint_cleanup_tl(TRint* gint);
  static void* Gled_runner_tl(Gled* gled);

  virtual void LockFltk()   {}
  virtual void UnlockFltk() {}

  static Gled* theOne;

#include "Gled.h7"
  ClassDef(Gled, 0);
}; // endclass Gled

extern Gled* gGled;

#endif
