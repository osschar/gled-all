// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Gled_H
#define Gled_Gled_H

// Includes
#include <Gled/GledTypes.h>

class Saturn;
class SaturnInfo;
class EyeInfo;
class ShellInfo;
class Forest;
class Mountain;
class Eye;

class GCondition;

class Fl_SWM_Manager;
class Fl_Window;
class Fl_Tile;
class Fl_Browser;

class TRint;
class TCanvas;

class Gled {

protected:
  void next_arg_or_die(list<char*>& args, list<char*>::iterator& i);

  SaturnInfo*	mSaturnInfo;	// X{G}
  Saturn*	mSaturn;	// X{G}
  bool		bIsSun;		// X{G}

  bool		bQuit;		// X{G}
  bool		bShowSplash;	// X{G}
  bool		bAutoSpawn;	// X{G}
  bool		bAllowMoons;	// X{G}

  bool		bRunRint;	// X{G}
  bool		bRintRunning;	// X{G}

  TString	mAuthDir;	 // X{GS}
  TString	mDefEyeIdentity; // X{GS}

  GCondition*	mExitCondVar;

public:
  Gled(list<char*>& args);
  virtual ~Gled();
  virtual void InitGledCore();

  void SetExitCondVar(GCondition* cond) { mExitCondVar = cond; }

  void SpawnSunOrSaturn();
  void SpawnSun();
  void SpawnSaturn();

  void        CheckAuthDir();
  const char* PubKeyFile(TString& id);
  const char* PrivKeyFile(TString& id);
  const char* GetPubKeyFile(TString& id, Bool_t use_exc=true);
  const char* GetPrivKeyFile(TString& id, Bool_t use_exc=true);

  bool IsIdentityInGroup(const char* id, const char* group);

  void WaitUntillQueensLoaded();
  void AllowMoonConnections();

  virtual void Run() {}
  virtual void Exit();

  virtual Int_t LoadLibSet(const Text_t* lib_set);
  virtual Int_t LoadLibSet(LID_t lid);
  virtual Int_t	AssertLibSet(const Text_t* lib_set);

  virtual void SetDebugLevel(Int_t d=0);

  virtual void output(const char* s);
  virtual void message(const char* s);
  virtual void warning(const char* s);
  virtual void error(const char* s);

  virtual void SpawnEye(const char* name, const char* title=0);
  virtual void SpawnEye(ShellInfo* si, const char* name, const char* title=0) {}

  static void* TRint_runner_tl(TRint* gint);
  static void* Gled_runner_tl(Gled* gled);

  static Gled* theOne;

  // Misc

  virtual void AddMTWLayout(const char* name, const char* layout) {}

#include "Gled.h7"
  ClassDef(Gled, 0)
}; // endclass Gled

#endif
