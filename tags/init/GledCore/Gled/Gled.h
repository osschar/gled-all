// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
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

  GCondition*	mExitCondVar;

public:
  Gled(list<char*>& args);
  virtual ~Gled();
  void SetExitCondVar(GCondition* cond) { mExitCondVar = cond; }

  void SpawnSunOrSaturn();
  void SpawnSun();
  void SpawnSaturn();

  void WaitUntillQueensLoaded();
  void AllowMoonConnections();

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

  static Gled* theOne;

#include "Gled.h7"
  ClassDef(Gled, 0)
}; // endclass Gled

#endif
