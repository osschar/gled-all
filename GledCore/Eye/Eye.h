// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Eye_H
#define Gled_Eye_H

#include <Glasses/ZGlass.h>
class ZQueen;
class SaturnInfo; class EyeInfo;

#include <Net/Ray.h>
#include <Stones/ZMIR.h>
class ShellInfo;

#include "OptoStructs.h"
class FTW_Shell;

#include <FL/Fl_Window.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_SWM.H>

#include <TMonitor.h>
#include <TSocket.h>
#include <TMessage.h>

typedef list<Fl_Window*>		lpFl_Window_t;
typedef list<Fl_Window*>::iterator	lpFl_Window_i;

class Eye {
protected:
  OptoStructs::hpZGlass2pZGlassImg_t	mGlass2ImgHash;

  Saturn*	mSaturn;	// X{G}
  ZQueen*	mQueen;
  EyeInfo*	mEyeInfo;	// X{G}

  FTW_Shell*	mShell;		// X{G}

  TSocket*	mSatSocket;
  TMessage	mMsg;
  Ray		mRay;		// so far a single one needed

  lpFl_Window_t mRedrawOnAnyRay;

public:
  Eye(UInt_t port, ID_t shell_id, const char* name="Eye", const char* title=0,
      const Fl_SWM_Manager* swm_copy=0);
  ~Eye();

  void show();

  // Basic ZGlassImg functionality
  OptoStructs::ZGlassImg* DemanglePtr(ZGlass* glass);

  // Socketing
  Int_t	Connect(UInt_t port);
  Int_t	Manage(int fd);

  void Send(TMessage* m);
  void Send(ZMIR& c);

  void CloseEye();

  void RegisterROARWindow(Fl_Window* w) { mRedrawOnAnyRay.push_back(w); }

#include "Eye.h7"
}; // endclass Eye

#endif
