// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_GledGUI_H
#define Gled_GledGUI_H

#include <Gled/Gled.h>
class ShellInfo;

#include <Gled/GThread.h>
#include <Gled/GCondition.h>

#include <FL/Fl_Window.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Input.H>

#include <FL/Fl_SWM.H>
#include <FL/Fl_OutputPack.H>

/**************************************************************************/

class GledGUI : public Gled, public Fl_Window, public Fl_SWM_Client {
private:

protected:
  struct Message {
    string	fMsg;
    Fl_Color	fCol;
    Message(const char* m, Fl_Color c=(Fl_Color)0) : fMsg(m), fCol(c) {}
  };
  typedef list<Message>		lMessage_t;

  Fl_OutputPack*  wOutPack;
  bool		  bGUIup;

  Fl_Value_Input* wDebugLevel;	// X{G}
  Fl_Input*	  wLibSetName;	// X{G}
  Fl_Input*	  wEyeName;	// X{G}

  GThread*	  mMessenger;
  GCondition	  mMsgCond;
  lMessage_t	  mMsgQueue;

  Int_t	          mNumShells;

public:
  GledGUI(list<char*>& args);
  virtual ~GledGUI();

  virtual void Run();
  virtual void Exit();

  virtual Int_t LoadLibSet(const Text_t* lib_set);

  virtual void SetDebugLevel(Int_t d=0);

  virtual void MessageLoop();
  virtual void PostMessage(const char* m, Fl_Color c=(Fl_Color)0);
  virtual void output(const char* s);
  virtual void message(const char* s);
  virtual void warning(const char* s);
  virtual void error(const char* s);

  virtual void SpawnEye(ShellInfo* si, const char* name, const char* title=0);

  TCanvas* NewCanvas(const Text_t* name, const Text_t* title,
		     int w, int h, int npx, int npy);

  virtual int handle(int ev);

  static GledGUI* theOne;

#include "GledGUI.h7"
}; // endclass GledGUI

#endif
