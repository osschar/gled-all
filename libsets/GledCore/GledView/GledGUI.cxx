// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GledGUI.h"
#include <Ephra/Saturn.h>
#include <Glasses/ShellInfo.h>
#include <Glasses/EyeInfo.h>
#include <Glasses/PupilInfo.h>
#include <Glasses/ZQueen.h>
#include <Glasses/ZFireQueen.h>
#include <Gled/GledNS.h>
#include <GledView/GledViewNS.h>
#include <Eye/Eye.h>

#include <TSystem.h>
#include <TSocket.h>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_OutputPack.H>
#include <FL/Fl_Tooltip.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Image.H>

extern void *GledCore_GLED_init_View;
extern void *GledCore_GLED_user_init_View;

/**************************************************************************/
// callback handlers
/**************************************************************************/

namespace
{
  void* MessageLoop_tl(GledGUI* gui)
  {
    gui->MessageLoop();
    return 0;
  }

  void quit_cb(Fl_Widget* o, void* ud)
  {
    GledGUI::theOne->Exit();
  }

  void debuglevel_cb(Fl_Value_Input* o, void* ud)
  {
    Gled::theOne->SetDebugLevel((Int_t)(o->value()));
  }

  void loadlibset_cb(Fl_Widget* o, void* ud)
  {
    Gled::theOne->LoadLibSet(GledGUI::theOne->GetLibSetName()->value());
  }

  void spawneye_cb(Fl_Widget* o, void* ud)
  {
    Gled::theOne->SpawnEye("GledCore", GledGUI::theOne->GetEyeName()->value());
  }

  void keeppos_cb(Fl_Button* o, Fl_OutputPack* ud)
  {
    ud->keep_pos(o->value());
  }
}

/**************************************************************************/
// GLED_LABEL_TYPE
/**************************************************************************/

namespace
{
  void fl_nosymbol_label(const Fl_Label* o, int X, int Y, int W, int H, Fl_Align align)
  {
    fl_font(o->font, o->size);
    fl_color((Fl_Color)o->color);
    fl_draw(o->value, X, Y, W, H, align, o->image, 0);
  }

  void fl_nosymbol_measure(const Fl_Label* o, int& W, int& H)
  {
    fl_font(o->font, o->size);
    fl_measure(o->value, W, H, 0);
    if (o->image) {
      if (o->image->w() > W) W = o->image->w();
      H += o->image->h();
    }
  }
}

/**************************************************************************/
// GledGUI
/**************************************************************************/

GledGUI* GledGUI::theOne = 0;

/**************************************************************************/

void GledGUI::build_gui()
{
  Fl_Group::current(this);

    // Top pack
  Fl_Pack* top_pack = new Fl_Pack(0,0,1,2);
  top_pack->type(FL_HORIZONTAL);

  wSwmResizer = new Fl_Button(0, 0, 2, 2, "@#3>>");
  wSwmResizer->labeltype(FL_SYMBOL_LABEL);

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
    wEyeName->value("FTW_Shell");
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
    b->labelcolor(168);
  }

  top_pack->end();

  // Output pack
  wOutPack = new Fl_OutputPack(0,2,60,28);
  wOutPack->base_skip_mod(-2);
  wOutPack->bg_color((Fl_Color)0x20202000);
  wOutPack->fg_color((Fl_Color)0xf0f0f000);

  keep_pos_flip->value(wOutPack->keep_pos());
  keep_pos_flip->callback((Fl_Callback*)keeppos_cb, wOutPack);

  Fl_Box* res_box = new Fl_Box(57,29,1,1);
  resizable(res_box);

  Fl_Group::current(0);
}

/**************************************************************************/

GledGUI::GledGUI() :
  Gled(), Fl_Window(60, 30, "Gled"),
  mMsgCond(GMutex::recursive),
  mNumShells(0),
  mSwmManager(0)
{
  end();

  if(theOne) {
    cerr <<"GledGUI::GledGUI trying to instantiate another object ...\n";
    exit(1);
  }
  theOne = this;

  bGuiUp = false;

  // Fix defaults set by Gled constructor.
  mLogFileName = "<null>";
  mRenderers   = "GL";

  // Call this early on, should be before first show().
  Fl::visual(FL_DOUBLE|FL_RGB);
}

void GledGUI::ParseArguments(Bool_t allow_daemon)
{
  Gled::ParseArguments(allow_daemon);

  // Defaults that can be overridden by options.
  int   swm_fs = 12, swm_vskip = 6, swm_hwidth = 0;
  int   font = 0;
  bool  start_iconized = false, no_msg_window = false;

  lStr_i i  = mArgs.begin();
  while (i != mArgs.end())
  {
    lStr_i start = i;

    if (*i == "-h" || *i == "-help" || *i == "--help" || *i == "-?")
    {
      cout << "\n"
        "GledGUI options:\n"
        "----------------\n"
        "  --swm   fs:dh:dw    specify font-size, vert-space and char width\n"
        "                      default: 12:6:0 (dw~0 means measure font)\n"
        "  --font  font-id     use fltk's font-id as default font\n"
        "  --iconize           iconize main window on start-up\n"
        "  --nomsgwin | --nomw start gled without the message window (consider '-log +')\n"
        "  --stereo            use stereo GL visuals by default\n";
      return;
    }
    else if (*i == "--swm")
    {
      next_arg_or_die(mArgs, i);
      sscanf(*i, "%d:%d:%d", &swm_fs, &swm_vskip, &swm_hwidth);
      mArgs.erase(start, ++i);
    }
    else if (*i == "--font")
    {
      next_arg_or_die(mArgs, i);
      font = atoi(*i);
      mArgs.erase(start, ++i);
    }
    else if (*i == "--iconize")
    {
      start_iconized = true;
      mArgs.erase(start, ++i);
    }
    else if (*i == "--nomw" || *i == "--nomsgwin")
    {
      no_msg_window = true;
      mArgs.erase(start, ++i);
    }
    else if (*i == "--stereo")
    {
      PupilInfo::sStereoDefault = true;
      mArgs.erase(start, ++i);
    }
    else
    {
      ++i;
    }
  }

  // Init starts here ... should be moved to InitLogging ?

  mSwmManager = new Fl_SWM_Manager(swm_fs, swm_vskip, swm_hwidth);

  GledViewNS::no_symbol_label = FL_FREE_LABELTYPE;

  Fl::set_labeltype((Fl_Labeltype)GledViewNS::no_symbol_label, fl_nosymbol_label, fl_nosymbol_measure);
  if (font)
  {
    printf("-font option currently disabled. There were problems overriding default font with fltk-1.1.7.\n");
    font = 0;
    // Fl::set_font((Fl_Font)FL_HELVETICA, (Fl_Font)font);

    // int n = Fl::set_fonts(); // load all iso8859-1 fonts
    // printf("Loaded %d fonts\n", n);
    // Fl::set_font((Fl_Font)FL_HELVETICA, "verdana");
  }

  fl_message_font((Fl_Font) font, swm_fs);

  Fl_Tooltip::size(mSwmManager->cell_fontsize() - 1);
  Fl_Tooltip::enable();	// enable tooltips
  Fl::visible_focus(0); // no focus for buttons ETC

  if (no_msg_window == false)
  {
    build_gui();
    swm_size_range = new SWM_Size_Range(40, 20, 200, 100);
    mSwmManager->adopt_window(this);
    set_swm_hotspot_cb(wSwmResizer);
    if(start_iconized)
      iconize();
    else
      show();
    bGuiUp = true;
  }
}

GledGUI::~GledGUI()
{
  delete mSwmManager;
}

void GledGUI::InitGledCore()
{
  Gled::InitGledCore();
  ((void(*)())GledCore_GLED_init_View)();
  ((void(*)())GledCore_GLED_user_init_View)();
}

/**************************************************************************/

void GledGUI::exec_gui_thread_request(GuiThreadRequest* gtr)
{
  gtr->fCondy.Lock();
  fGTRQueueMoo.Lock();
  fGTRQueue.push_back(gtr);
  fGTRQueueMoo.Unlock();
  Fl::awake();
  printf("GledGUI::exec_gui_thread_request waiting ...\n");
  gtr->fCondy.Wait();
}

void GledGUI::Run()
{
  // Runs in dedicated thread spawned from gled.cxx.
  // ALL gui (also for eyes and therefore for pupils) runs through this loop.

  // printf("GledGUI::Run entering GUI event loop.\n");

  Fl::lock();		// init thread support

  if (bGuiUp)
  {
    mMessenger = new GThread("GledGUI-MessageLoop",
                             (GThread_foo) MessageLoop_tl, this,
                             false);
    mMessenger->Spawn();
  }

  while (!bQuit)
  {
    fGTRQueueMoo.Lock();
    while (!fGTRQueue.empty())
    {
      GuiThreadRequest* gtr = fGTRQueue.front(); fGTRQueue.pop_front();
      gtr->fCondy.Lock();
      gtr->Execute();
      gtr->fCondy.Signal();
      gtr->fCondy.Unlock();
    }
    fGTRQueueMoo.Unlock();

    Fl::wait(10);
  }

  Fl::unlock();

  if (bGuiUp)
  {
    mMsgCond.Lock();
    bGuiUp = false;
    mMsgCond.Signal();
    mMsgCond.Unlock();
    mMessenger->Join();
    delete mMessenger; mMessenger = 0;
    mMsgQueue.clear();
  }
  // printf("GledGUI::Run exiting GUI event loop.\n");
}

void GledGUI::ExitVirtual()
{
  // Shutdown Eyes, GUI ... then Gled

  Gled::ExitVirtual();

  Fl::lock();
  Fl::awake();
  Fl::unlock();
}

/**************************************************************************/

Int_t GledGUI::LoadLibSet(const Text_t* lib_set)
{
  Int_t ret = Gled::LoadLibSet(lib_set);
  return ret ? ret : GledViewNS::LoadSoSet(lib_set);
}

/**************************************************************************/

void GledGUI::SetDebugLevel(Int_t d)
{
  if (d < 0) d = 0;
  Gled::SetDebugLevel(d);
  if (bGuiUp) wDebugLevel->value(d);
}

/**************************************************************************/
// Info Stream methods
/**************************************************************************/

// GUI controlled by separate thread driven by GledGUI::Run method
// It spawns another thread that does MessageLoop:

void GledGUI::MessageLoop()
{
  while (bGuiUp)
  {
    mMsgCond.Lock();
    mMsgCond.Wait();
    if (mMsgQueue.begin() != mMsgQueue.end())
    {
      mMsgCond.Unlock();
      Fl::lock();
      mMsgCond.Lock();
      while (mMsgQueue.begin() != mMsgQueue.end())
      {
	Message& msg = mMsgQueue.front();
	wOutPack->add_line(msg.fMsg.Data(), msg.fCol);
	mMsgQueue.pop_front();
      }
      wOutPack->redraw();
      Fl::awake();
      Fl::unlock();
    }
    mMsgCond.Unlock();
  }
  printf("exiting GledGUI::MessageLoop()\n");
}

void GledGUI::PostMessage(const char* m, Fl_Color c)
{
  mMsgCond.Lock();
  mMsgQueue.push_back(Message(m, c));
  mMsgCond.Signal();
  mMsgCond.Unlock();
}

void GledGUI::info(const char* s)
{
  Gled::info(s);
  if(bGuiUp) PostMessage(s);
}

void GledGUI::message(const char* s)
{
  Gled::message(s);
  if(bGuiUp) PostMessage(s, FL_CYAN);
}


void GledGUI::warning(const char* s)
{
  Gled::warning(s);
  if(bGuiUp) PostMessage(s, FL_YELLOW);
}

void GledGUI::error(const char* s)
{
  Gled::error(s);
  if(bGuiUp) PostMessage(s, FL_RED);
}

/**************************************************************************/
/**************************************************************************/

EyeInfo* GledGUI::SpawnEye(EyeInfo* ei, ZGlass* ud,
			   const TString& libset, const TString& eyector)
{
  // Wrapper for eye spawning from CINT.

  static const Exc_t _eh("GledGUI::SpawnEye ");

  struct EyeCreationRequest : public GuiThreadRequest
  {
    // Input:
    EyeInfo::EyeCreator_foo     fECFoo;
    EyeInfo                    *fEyeInfo;
    ZGlass                     *fUserData;
    // Internals
    TSocket                    *fSocket;
    // Output
    Eye                        *fEye;

    EyeCreationRequest(EyeInfo::EyeCreator_foo foo, EyeInfo* ei, ZGlass* ud) :
      fECFoo(foo), fEyeInfo(ei), fUserData(ud), fSocket(0), fEye(0) {}

    virtual ~EyeCreationRequest() {}

    virtual void Execute()
    {
      try
      {
	fEye = fECFoo(fSocket, fEyeInfo, fUserData);
      }
      catch(Exc_t& exc)
      {
	delete fSocket;
	ISerr(_eh + "Eye creation failed: '" + exc + "'.");
      }
    }

    EyeInfo* TrueEyeInfo() { return fEye ? fEye->GetEyeInfo() : 0; }
  };

  bool wipe_ei = false;

  TString eye_name = GForm("%s@%s", mDefEyeIdentity.Data(), gSystem->HostName());

  if (ei == 0)
  {
    ei = new EyeInfo(eye_name);
    wipe_ei = true;
  }
  if (strlen(ei->GetLogin()) == 0)
    ei->SetLogin(mDefEyeIdentity);

  if (ud==0 && libset == "GledCore" && eyector == "FTW_Shell")
  {
    ZFireQueen* fq = mSaturn->GetFireQueen();
    ShellInfo* si = new ShellInfo
      (GForm("Shell[%d] of %s", ++mNumShells, eye_name.Data()),
             "Created by GledGUI");
    fq->CheckIn(si); fq->Add(si);
    si->MakeDefSubShell();
    ud = si;
  }

  TString foo_name("EyeCreator_"); foo_name += libset + "_" + eyector;
  long* p2foo = (long*) GledNS::FindSymbol(foo_name);
  if (!p2foo)
  {
    ISerr(_eh +"can't find symbol '"+ foo_name +"'.");
    return 0;
  }

  EyeCreationRequest ecr((EyeInfo::EyeCreator_foo)(*p2foo), ei, ud);
  // To connect Eye via localhost inet socket (server must be running):
  //   ecr.fSocket = new TSocket("localhost", mSaturnInfo->GetServerPort());
  // To connect via unix unnamed socket:
  ecr.fSocket = mSaturn->MakeSocketPairAndAccept("saturn-eye");
  exec_gui_thread_request(&ecr);

  if (wipe_ei)
    delete ei;

  return ecr.TrueEyeInfo();
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

/**************************************************************************/

void GledGUI::LockFltk()
{
  Fl::lock();
}

void GledGUI::UnlockFltk()
{
  Fl::awake();
  Fl::unlock();
}
