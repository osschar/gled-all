// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GledGUI_H
#define GledCore_GledGUI_H

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

class GledGUI : public Gled, public Fl_Window, public Fl_SWM_Client
{
protected:
  struct Message
  {
    TString	fMsg;
    Fl_Color	fCol;
    Message(const char* m, Fl_Color c=(Fl_Color)0) : fMsg(m), fCol(c) {}
  };
  typedef list<Message>		lMessage_t;

  Fl_OutputPack*  wOutPack;

  Bool_t	  bGuiUp;

  Fl_Button*	  wSwmResizer;
  Fl_Value_Input* wDebugLevel;	// X{g}
  Fl_Input*	  wLibSetName;	// X{g}
  Fl_Input*	  wEyeName;	// X{g}

  GThread*	  mMessenger;
  GCondition	  mMsgCond;
  lMessage_t	  mMsgQueue;

  Int_t	          mNumShells;

  Fl_SWM_Manager* mSwmManager;  // X{g}

  void build_gui();

  struct GuiThreadRequest
  {
    GCondition fCondy;

    virtual ~GuiThreadRequest() {}
    virtual void Execute() = 0;
  };
  std::list<GuiThreadRequest*> fGTRQueue;
  GMutex                       fGTRQueueMoo;

  void exec_gui_thread_request(GuiThreadRequest* gtr);

public:
  GledGUI();
  virtual void ParseArguments();
  // virtual void InitLogging();
  virtual void InitGledCore();
  virtual ~GledGUI();

  virtual void Run();
  virtual void ExitVirtual();

  virtual Int_t LoadLibSet(const Text_t* lib_set);

  virtual void SetDebugLevel(Int_t d=0);

  virtual void MessageLoop();
  virtual void PostMessage(const char* m, Fl_Color c=(Fl_Color)0);
  virtual void output(const char* s);
  virtual void message(const char* s);
  virtual void warning(const char* s);
  virtual void error(const char* s);

  virtual Bool_t HasGUILibs() const { return true; }

  virtual EyeInfo* SpawnEye(EyeInfo* ei, ZGlass* ud,
			    const TString& libset, const TString& eyector);

  virtual TCanvas*
  NewCanvas(const Text_t* name ="Gled Canvas",
            const Text_t* title="Default Gled Canvas",
	    int w=640, int h=480, int npx=1, int npy=1);

  virtual int handle(int ev);

  virtual void LockFltk();
  virtual void UnlockFltk();

  static GledGUI* theOne;

#include "GledGUI.h7"
}; // endclass GledGUI

#endif
