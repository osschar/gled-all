// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_FTW_Shell_H
#define Gled_FTW_Shell_H

#include <Gled/GledTypes.h>
#include <Glasses/ShellInfo.h>
#include "OptoStructs.h"
#include "FTW.h"
class FTW_Leaf;
class FTW_Nest;

#include <FL/Fl_SWM.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Pack.H>

class Fl_OutputPack;

class FTW_Shell : public OptoStructs::A_View,
		  public Fl_Window,
		  public Fl_SWM_Manager, public Fl_SWM_Client
{
public:
  enum		MType_e { MT_std=0, MT_err, MT_wrn, MT_msg };
protected:
  Eye*		mEye;		// X{G}
  ShellInfo*	mShellInfo;	// X{G}

  FTW::NestAm*	 pNestAm;

  Fl_Pack*	 wMainPack;
  FTW_Nest*	 mNest;		// X{G}
  Fl_OutputPack* wOutPack;

  FTW::Source_Selector*	mSource; // X{G}
  FTW::Sink_Selector*	mSink;   // X{G}

  void label_shell();

public:
  FTW_Shell(OptoStructs::ZGlassImg* img, const Fl_SWM_Manager* swm_copy);
  virtual ~FTW_Shell();

  virtual void Absorb_Change(LID_t lid, CID_t cid);
  // !!!! change window label !!!!
  // virtual void Absorb_LinkChange(LID_t lid, CID_t cid) {}
  // virtual void Absorb_Delete() {}

  void X_SetSource(FTW::Locator& loc);
  void X_SetSink(FTW::Locator& loc);
  void X_ExchangeSourceAndSink();

  void X_SetBeta(FTW::Locator& loc);
  void X_SetGamma(FTW::Locator& loc);

  void X_SetLink(FTW::Locator& target);
  void X_ClearLink(FTW::Locator& target);
  void X_Yank(FTW::Locator& target);
  void X_Push(FTW::Locator& target);
  void X_Unshift(FTW::Locator& target);
  void X_Insert(FTW::Locator& target);
  void X_Pop(FTW::Locator& target);
  void X_Shift(FTW::Locator& target);
  void X_Remove(FTW::Locator& target);

  void Y_SendMessage(const char* msg);

  void ExecContextCall(FTW::Locator& alpha, GledNS::MethodInfo* cmi);
  void ExportToInterpreter(FTW::Locator& loc, const char* varname);

  FTW_Nest* SpawnNest(OptoStructs::ZGlassImg* img);
  void	    SpawnMTW_View(OptoStructs::ZGlassImg* img);

  void LocatorMenu(FTW::Locator& loc, int x, int y);

  void Message(const char* msg, MType_e t=MT_std);
  void Message(const string& msg, MType_e t=MT_std);

  int handle(int ev);

#include "FTW_Shell.h7"
}; // endclass FTW_Shell

#endif
