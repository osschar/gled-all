// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Eye_H
#define GledCore_Eye_H

#include <Glasses/ZGlass.h>
#include "OptoStructs.h"

class ZQueen;
class SaturnInfo;
class EyeInfo;
class ShellInfo;
class ZMIR;

class FTW_Shell;
class Fl_SWM_Manager;
class Fl_Window;

class TSocket;
class TMessage;


class Eye {
protected:
  typedef list<Fl_Window*>		lpFl_Window_t;
  typedef list<Fl_Window*>::iterator	lpFl_Window_i;


  OptoStructs::hpZGlass2pZGlassImg_t	mGlass2ImgHash;

  Saturn*	mSaturn;	// X{g}
  SaturnInfo*	mSaturnInfo;	// X{g}
  ZQueen*	mQueen;		// X{g}
  EyeInfo*	mEyeInfo;	// X{g}
  FTW_Shell*	mShell;		// X{g}

  TSocket*	mSatSocket;

  lpFl_Window_t mRedrawOnAnyRay;
  bool		bBreakManageLoop;

public:
  Eye(UInt_t port, TString identity, ID_t shell_id,
      const char* name="Eye", const char* title=0,
      const Fl_SWM_Manager* swm_copy=0);
  ~Eye();

  void show();

  // Basic ZGlassImg functionality
  OptoStructs::ZGlassImg* DemanglePtr(ZGlass* glass);
  OptoStructs::ZGlassImg* DemangleID(ID_t id);
  void RemoveImage(OptoStructs::ZGlassImg* img);

  // Socketing
  Int_t	Manage(int fd);

  void Send(TMessage* m);
  void Send(ZMIR& c);

  void CloseEye();

  void RegisterROARWindow(Fl_Window* w) { mRedrawOnAnyRay.push_back(w); }
  void BreakManageLoop() { bBreakManageLoop = true; }

#include "Eye.h7"
}; // endclass Eye

#endif
