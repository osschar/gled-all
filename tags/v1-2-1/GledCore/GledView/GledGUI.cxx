// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GledGUI.h"
#include <Ephra/Saturn.h>
#include <Glasses/ShellInfo.h>
#include <Gled/GledNS.h>
#include <GledView/GledViewNS.h>
#include <Eye/Eye.h>

#include <glue/GledCore_View_LibSet.h>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_OutputPack.H>
#include <FL/Fl_Tooltip.H>
#include <FL/x.H>

#include <TROOT.h>
#include <TVirtualX.h>
#include <TGX11.h>
#include <TCanvas.h>

// #include <GL/glx.h>
#include <X11/Xlib.h>

/**************************************************************************/
// callback handlers
/**************************************************************************/

namespace {

  void* tf_MessageLoop(GledGUI* gui) {
    gui->MessageLoop();
    GThread::Exit();
    return 0;
  }

  void quit_cb(Fl_Widget* o, void* ud) { GledGUI::theOne->Exit(); }

  void debuglevel_cb(Fl_Value_Input* o, void* ud)
  { Gled::theOne->SetDebugLevel((Int_t)(o->value())); }

  void loadlibset_cb(Fl_Widget* o, void* ud)
  { Gled::theOne->LoadLibSet(GledGUI::theOne->GetLibSetName()->value()); }

  void spawneye_cb(Fl_Widget* o, void* ud)
  { Gled::theOne->SpawnEye(GledGUI::theOne->GetEyeName()->value()); }

  void keeppos_cb(Fl_Button* o, Fl_OutputPack* ud)
  { ud->keep_pos(o->value()); }
}

/**************************************************************************/
// GledGUI
/**************************************************************************/

GledGUI* GledGUI::theOne = 0;

/**************************************************************************/

GledGUI::GledGUI(list<char*>& args) :
  Gled(args),
  Fl_Window(60, 30, "Gled"),
  bGUIup(false),
  mMsgCond(GMutex::recursive)
{
  if(theOne) {
    cerr <<"GledGUI::GledGUI trying to instantiate another object ...\n";
    exit(1);
  }
  theOne = this;

  char* rnr_string = "GL";
  int swm_fs = 12, swm_vskip = 6, swm_hwidth = 0;
  list<char*>::iterator i = args.begin();
  while(i != args.end()) {
    list<char*>::iterator start = i;

    if(strcmp(*i, "-h")==0 || strcmp(*i, "-help")==0 ||
       strcmp(*i, "--help")==0 || strcmp(*i, "-?")==0)
      {
	cout <<
	  "GledGUI options:\n"
	  "  -swm	fs:dh:dw	specify font-size, vert-space and char width\n"
	  "			default 12:6:0 (dw~0 means measure font)\n"
	  "  -rnr <r1>:<r2>:...	specify which rendering libraries to load\n"
	  "			default GL\n";

	return;
      }

    else if(strcmp(*i, "-swm")==0) {
      next_arg_or_die(args, i);
      int num = sscanf(*i, "%d:%d:%d", &swm_fs, &swm_vskip, &swm_hwidth);
      args.erase(start, ++i);
    }

    else if(strcmp(*i, "-rnr")==0) {
      next_arg_or_die(args, i);
      rnr_string = *i;
      args.erase(start, ++i);
    }

    else {
      ++i;
    }

  }

  libGledCore_GLED_init_View();

  if(rnr_string) {
    while(rnr_string && *rnr_string!=0) {
      char* col = index(rnr_string, ':');
      if(col) *(col++) = 0;
      GledViewNS::AddRenderer(rnr_string);
      if(col) rnr_string = col;
      else    break;
    }
  }

  // Top pack
  Fl_Pack* top_pack = new Fl_Pack(0,0,1,2);
  top_pack->type(FL_HORIZONTAL);

  Fl_Button* swm_butt = new Fl_Button(0, 0, 2, 2, "@#3>>");
  swm_butt->labeltype(FL_SYMBOL_LABEL);

  {
    Fl_Group* groo = new Fl_Group(0,0,12,2);
    Fl_Button* b = new Fl_Button(0,0,12,1, "Load LibSet");
    b->callback((Fl_Callback*)loadlibset_cb);
    wLibSetName = new Fl_Input(0,1,12,1);
    wLibSetName->callback((Fl_Callback*)loadlibset_cb);
    wLibSetName->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);
    groo->end();
  }
  {
    Fl_Group* groo = new Fl_Group(0,0,12,2);
    Fl_Button* b = new Fl_Button(0,0,12,1, "Spawn Eye");
    b->callback((Fl_Callback*)spawneye_cb);
    wEyeName = new Fl_Input(0,1,12,1);
    wEyeName->callback((Fl_Callback*)spawneye_cb);
    wEyeName->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);
    groo->end();
  }
  Fl_Button* keep_pos_flip;
  {
    Fl_Group* groo = new Fl_Group(0,0,9,2);
    wDebugLevel = new Fl_Value_Input(0,0,3,1, "Dbg Lvl");
    wDebugLevel->callback((Fl_Callback*)debuglevel_cb);
    wDebugLevel->align(FL_ALIGN_RIGHT);
    wDebugLevel->range(0,10); wDebugLevel->step(1);
    wDebugLevel->value(G_DEBUG);
    keep_pos_flip = new Fl_Check_Button(0,1,8,1, "Keep Pos");
    keep_pos_flip->align(FL_ALIGN_INSIDE|FL_ALIGN_RIGHT);
    groo->end();
  }
  {
    Fl_Button* b = new Fl_Button(0,0,16,2,"EatFlamingDeath");
    b->callback(quit_cb);
    b->labelcolor(168); b->tooltip("are tooltips really working?");
  }

  top_pack->end();

  // Output pack
  wOutPack = new Fl_OutputPack(0,2,60,28);
  wOutPack->base_skip_mod(-4);
  wOutPack->bg_color((Fl_Color)0x20202000);
  wOutPack->fg_color((Fl_Color)0xf0f0f000);

  keep_pos_flip->value(wOutPack->keep_pos());
  keep_pos_flip->callback((Fl_Callback*)keeppos_cb, wOutPack);

  Fl_Box* res_box = new Fl_Box(57,29,1,1);
  resizable(res_box);
  end();

  swm_size_range = new SWM_Size_Range(40, 20, 200, 100);
  Fl_SWM_Manager* swm = new Fl_SWM_Manager(swm_fs, swm_vskip, swm_hwidth);
  swm->adopt_window(this);
  set_swm_hotspot_cb(swm_butt);

  Fl_Tooltip::size(swm_manager->cell_fontsize() - 1);

  show();
}

GledGUI::~GledGUI()
{
  delete swm_manager;
}

/**************************************************************************/

void GledGUI::Exit()
{
  // Shutdown Eyes, GUI ... then Gled

  // ... !!!
  bGUIup = false;
  Gled::Exit();
  
  Fl::lock();
  Fl::awake();
  Fl::unlock();
}

void GledGUI::Run()
{
  // Runs in dedicated thread spawned from gled.cxx.
  // ALL gui (also for eyes and therefore for pupils) runs through this loop.

  ISmess("GledGUI::Run starting GUI");
  Fl::lock();		// init thread support
  Fl::visible_focus(0); // no focus for buttons ETC
  Fl_Tooltip::enable();	// enable tooltips
  mMessenger = new GThread((thread_f)tf_MessageLoop, this, false);
  mMessenger->Spawn();
  bGUIup = true;
  while(!bQuit) Fl::wait();
  mMessenger->Cancel();
  mMessenger->Join();
  delete mMessenger; mMessenger = 0;
  Fl::unlock();
  ISmess("GledGUI::Run exiting GUI event loop");
}

/**************************************************************************/

Int_t GledGUI::LoadLibSet(const Text_t* lib_set)
{
  Int_t ret = Gled::LoadLibSet(lib_set);
  return ret ? ret : GledViewNS::LoadSoSet(lib_set);
}

/**************************************************************************/

void GledGUI::SetDebugLevel(Int_t d) {
  if(d<0) d=0;
  Gled::SetDebugLevel(d);
  wDebugLevel->value(d);
}

/**************************************************************************/
// Info Stream methods
/**************************************************************************/

// GUI controlled by separate thread driven by GledGUI::Run method
// It spawns another thread that does MessageLoop.
// mechanisms ... and probably notify another thread to actually
// deliver messages to gui. Here we assume all will be OK ...

void GledGUI::MessageLoop()
{
  while(1) {
    mMsgCond.Lock();
    mMsgCond.Wait();
    if(mMsgQueue.begin() != mMsgQueue.end()) {
      mMsgCond.Unlock();
      Fl::lock();
      mMsgCond.Lock();
      while(mMsgQueue.begin() != mMsgQueue.end()) {
	Message& msg = mMsgQueue.front();
	wOutPack->add_line(msg.fMsg, msg.fCol);
	mMsgQueue.pop_front();
      }
      wOutPack->redraw();
      Fl::awake();
      Fl::unlock();
    }
    mMsgCond.Unlock();
  }
}

void GledGUI::PostMessage(const char* m, Fl_Color c)
{
  mMsgCond.Lock();
  mMsgQueue.push_back(Message(m, c));
  mMsgCond.Signal();
  mMsgCond.Unlock();
}

void GledGUI::output(const char* s) {
  if(!bGUIup) { Gled::output(s); return; }
  PostMessage(s);
}

void GledGUI::message(const char* s) {
  if(!bGUIup) { Gled::message(s); return; }
  PostMessage(s, FL_CYAN);
}


void GledGUI::warning(const char* s) {
  if(!bGUIup) { Gled::warning(s); return; }
  PostMessage(s, FL_YELLOW);
}

void GledGUI::error(const char* s) {
  if(!bGUIup) { Gled::error(s); return; }
  PostMessage(s, FL_RED);
}

/**************************************************************************/
/**************************************************************************/

void GledGUI::SpawnEye(ShellInfo* si, const char* name, const char* title)
{
  // Wrapper for eye spawning from CINT.
  // I really forgot why XLockDisplay is here. It sure
  // helped once (note that ROOT has different display than fltk/gled_gui).
  // Fl locks are just.

  Display* rd = (Display*)(dynamic_cast<TGX11*>(gVirtualX)->GetDisplay());
  XLockDisplay(rd);

  Eye* e = new Eye(mSaturn->GetSaturnInfo()->GetServerPort(), si->GetSaturnID(),
		   name, title, swm_manager);

  Fl::lock();
  e->show();
  Fl::awake();
  Fl::unlock();

  XUnlockDisplay(rd);
}

/**************************************************************************/

TCanvas* GledGUI::NewCanvas(const Text_t* name, const Text_t* title,
			    int w, int h, int npx, int npy)
{
  XSync(fl_display, False);
  Display* rd = (Display*)(dynamic_cast<TGX11*>(gVirtualX)->GetDisplay());
  XLockDisplay(rd);

  TCanvas* c = new TCanvas(name, title, w, h);

  if(c && (npx>1 || npy>1)) {
    c->Divide(npx,npy);
    c->cd(1);
  }
  c->Update();

  XSync(rd, False);
  XUnlockDisplay(rd);

  return c;
}

/**************************************************************************/

int GledGUI::handle(int ev)
{
  if(ev == FL_SHORTCUT && Fl::event_key() == FL_Escape && parent() == 0) {
    iconize();
    return 1;
  }
  return Fl_Window::handle(ev);
}
