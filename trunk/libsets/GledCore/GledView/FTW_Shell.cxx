// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include <Glasses/ZQueen.h>
#include <Glasses/ZFireQueen.h>
#include <Ephra/Saturn.h>
#include <Gled/GledNS.h>
#include <GledView/GledGUI.h>

#include <Glasses/ShellInfo.h>
#include <Glasses/NestInfo.h>

#include "FTW_Shell.h"
#include "FTW_Leaf.h"
#include "FTW_Branch.h"
#include "FTW_Ant.h"
#include "FTW_Nest.h"
#include "MTW_ClassView.h"
#include "MTW_MetaView.h"

#include "MCW_View.h"

#include <FL/Fl_OutputPack.H>

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/fl_ask.H>

//#include <TSocket.h>
#include <TROOT.h>

namespace GNS  = GledNS;
namespace GVNS = GledViewNS;
namespace OS   = OptoStructs;
namespace FGS  = FltkGledStuff;

using namespace FTW;

/**************************************************************************/
// creator foo
/**************************************************************************/

FTW_Shell* FTW_Shell::Create_FTW_Shell(TSocket* sock,
				       EyeInfo* ei,
				       ZGlass* ud_lens)
{
  static const Exc_t _eh("FTW_Shell::Create_FTW_Shell ");

  GledGUI* ggui = GledGUI::theOne;
  if(ggui == 0) throw _eh + "GledGUI not accessible.";

  ShellInfo* si = dynamic_cast<ShellInfo*>(ud_lens);
  if(si == 0) throw _eh + "user-data is not ShellInfo.";

  Fl::lock();

  FTW_Shell* shell = new FTW_Shell(sock, ei, ggui->GetSwmManager());
  shell->SetImg(shell->DemanglePtr(ud_lens));
  shell->mShellInfo = si;

  shell->_bootstrap();

  shell->show();

  shell->_bootstrap_subshells();

  Fl::awake();
  Fl::unlock();

  shell->InstallFdHandler();
  return shell;
}

void *EyeCreator_GledCore_FTW_Shell = (void*)FTW_Shell::Create_FTW_Shell;

/**************************************************************************/

namespace
{
  /**************************************************************************/
  // Configs
  /**************************************************************************/

  int min_W = 30;
  int max_W = 512;
  int max_H = 256;

  int min_H = 9;
  int min_canvas_H = 6;

  TString _meth_no_cat(" method not found in catalog");

  /**************************************************************************/
  // GUI low-level stuff
  /**************************************************************************/

  // New menu
  //----------

  void new_menu_cb(Fl_Menu_Button* b, void* what)
  {
    FTW_Shell* shell = FGS::grep_parent<FTW_Shell*>(b);

    switch(GNS::CastVoidPtr2ID(what)) {

    case 1: { // Nest
      FID_t fid = GledNS::FindClassID("NestInfo");
      SubShellInfo* ss = (SubShellInfo*) GledNS::ConstructLens(fid);
      ss->SetName("Nest");
      shell->SpawnSubShell(ss, true);
      delete ss;
      break;
    }

    case 2: { // Pupil
      FID_t fid = GledNS::FindClassID("PupilInfo");
      SubShellInfo* ss = (SubShellInfo*) GledNS::ConstructLens(fid);
      ss->SetName("Pupil");
      shell->SpawnSubShell(ss, true);
      delete ss;
      break;
    }

    } // end switch
  }

  Fl_Menu_Item s_New_Menu[] = {
    { "Nest",  0, (Fl_Callback*) new_menu_cb, (void*)1 },
    { "Pupil", 0, (Fl_Callback*) new_menu_cb, (void*)2 },
    {0}
  };


  // View menu
  //----------

  void view_menu_cb(Fl_Menu_Button* b, void* what)
  {
    FTW_Shell* shell = FGS::grep_parent<FTW_Shell*>(b);
    bool on_p = b->mvalue()->value();
    switch(GNS::CastVoidPtr2ID(what)) {
    case 1: shell->SourceVis(on_p); break;
    case 2: shell->SinkVis(on_p);   break;
    }
  }

  Fl_Menu_Item s_View_Menu[] = {
    { "Source",  0, (Fl_Callback*) view_menu_cb, (void*)1, FL_MENU_TOGGLE|FL_MENU_VALUE },
    { "Sink",    0, (Fl_Callback*) view_menu_cb, (void*)2, FL_MENU_TOGGLE|FL_MENU_VALUE },
    {0}
  };


  // Message sending callback
  //--------------------------

  void msg_send_cb(Fl_Input* w, FTW_Shell* s)
  {
    try {
      s->Y_SendMessage(w->value());
    }
    catch(Exc_t& exc) {
      s->Message(exc.Data(), Eye::MT_err);
    }
    w->redraw();
  }
}

/**************************************************************************/
/**************************************************************************/
// FTW_Shell
/**************************************************************************/
/**************************************************************************/

FTW_Shell::FTW_Shell(TSocket* sock, EyeInfo* ei, const Fl_SWM_Manager* swm_copy) :
  Eye(sock, ei),
  OS::A_View(0),
  Fl_Window(1, 1),
  Fl_SWM_Manager(swm_copy), Fl_SWM_Client()
{
  mSubShellCount = 0;
}

FTW_Shell::~FTW_Shell()
{
  if(mSatSocket) {
    Fl::remove_fd(mSatSocketFd);
  }
}

void FTW_Shell::_bootstrap()
{
  int def_w        = mShellInfo->GetDefW();
  int def_sshell_h = mShellInfo->GetDefSShellH();
  int top_h        = 3;
  int msgout_h     = mShellInfo->GetMsgOutH();

  bool src_on_p    = mShellInfo->GetDefSourceVis();
  bool snk_on_p    = mShellInfo->GetDefSinkVis();

  int src_snk_h    = (src_on_p ? 2 : 0) + (snk_on_p ? 2 : 0);

  int sum_h = top_h + src_snk_h + def_sshell_h + 2 + msgout_h;

  size(def_w, sum_h);

  begin();

  wMainPack = new Fl_Pack(0, 0, w(), h());
  wMainPack->type(FL_VERTICAL);

  {
    wMenuPack = new Fl_Pack(0, 0, w(), 2);
    wMenuPack->type(FL_HORIZONTAL);

    Fl_Button* b_swm = new Fl_Button(0, 0, 2, 2, "@#3>>");
    b_swm->box((Fl_Boxtype)GVNS::menubar_box);
    b_swm->down_box((Fl_Boxtype)GVNS::menubar_box);
    b_swm->labeltype(FL_SYMBOL_LABEL);
    set_swm_hotspot_cb(b_swm);

    new FGS::MenuBox(s_New_Menu,  4, 2, "New");
    new FGS::MenuBox(s_View_Menu, 4, 2, "View");
    if(!src_on_p) s_View_Menu[0].flags ^= FL_MENU_VALUE;
    if(!snk_on_p) s_View_Menu[1].flags ^= FL_MENU_VALUE;

    FGS::LensRepNameBox* self = new FGS::LensRepNameBox(fImg, 0, 0, max_W, 2);
    self->box((Fl_Boxtype)GVNS::menubar_box);
    self->color(fl_rgb_color(220,200,200));

    wMenuPack->end();
    wMenuPack->resizable(0);
  }

  mSource = new FTW::Source_Selector(this, 0, 0, 6, "Source");
  if(!src_on_p) mSource->hide();
  mSink   = new FTW::Sink_Selector(this, 0, 0, 6, "Sink");
  if(!snk_on_p) mSink->hide();

  Fl_Box* nest_pre = new Fl_Box(FTW::separator_box,0,0,1,1, "Swallowed SubShell:");
  nest_pre->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  nest_pre->labelfont(nest_pre->labelfont() + FL_BOLD);
  nest_pre->color(fl_rgb_color(200,220,200));

  wCanvas = new Fl_Group(0, top_h + src_snk_h, w(), def_sshell_h);
  wCanvas->end();
  mCurSubShell = 0;

  // Message Input/Output

  // new FGS::PackEntryCollapsor("Message input/output:");
  {
    wMsgPack = new Fl_Pack(0, 0, w(), msgout_h + 2);
    wMsgPack->type(FL_VERTICAL);
    { // Top matter
      Fl_Group* inpp = new Fl_Group(0, 0, w(), 1);

      Fl_Box* msg_pre = new Fl_Box(FTW::separator_box,0,0,14,1,
				   "Message input/output:");
      msg_pre->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
      msg_pre->labelfont(msg_pre->labelfont() + FL_BOLD);
      msg_pre->color(fl_rgb_color(200,220,200));

      Fl_Group::current(0);
      MTW_Layout lout(0);
      lout.GetLaySpecs()->value("ShellInfo(MessageRecipient[16],MsgOutH)");
      lout.Parse(cell_w());
      Fl_Group::current(inpp);

      MTW_ClassView* mtw = new MTW_ClassView(fImg, this);
      mtw->BuildByLayout(&lout);
      mtw->Labelofy();
      mtw->position(msg_pre->w(), 0);

      int xbeg = mtw->x() + mtw->w();
      Fl_Box* xb = new Fl_Box(FTW::separator_box, xbeg, 0, w()-xbeg, 1, "");
      xb->color(fl_rgb_color(200,220,200));

      inpp->end();
      inpp->resizable(xb);
    }
    { // Output
      wOutPack = new Fl_OutputPack(0, 0, w(), msgout_h);
      //wOutPack->base_size_mod(-1);
      wOutPack->base_skip_mod(-4);
      wOutPack->bg_color((Fl_Color)0x20202000);
      wOutPack->fg_color((Fl_Color)0xf0f0f000);
    }
    { // Input
      Fl_Input* flinp = new Fl_Input(0,0,w(),1);
      flinp->callback((Fl_Callback*)msg_send_cb, this);
      flinp->when(FL_WHEN_ENTER_KEY_ALWAYS);
    }
    wMsgPack->end();
    wMsgPack->resizable(0);
  }

  wMainPack->end();
  end();

  wMainPack->resizable(wCanvas);
  resizable(wMainPack);

  swm_size_range = new SWM_Size_Range(min_W, min_H + min_canvas_H + wOutPack->h(),
				      max_W, max_H);
  adopt_window(this);

  label_shell();
}


void FTW_Shell::_bootstrap_subshells()
{
  // SubShells list.
  OS::ZGlassImg* sshells = DemanglePtr(mShellInfo->GetSubShells());

  OS::lpZGlassImg_t* imgs = sshells->GetElementImgs();
  for(OS::lpZGlassImg_i i=imgs->begin(); i!=imgs->end(); ++i) {
    spawn_subshell(*i, (*i)->fLens != mShellInfo->GetDefSubShell());
  }

  // Default SubShell.
  if(mShellInfo->GetDefSubShell() != 0) {
    OS::ZGlassImg* dssi = DemanglePtr(mShellInfo->GetDefSubShell());
    set_canvased_subshell(dssi);
  }
}

/**************************************************************************/
// Eye
/**************************************************************************/

void FTW_Shell::InstallFdHandler()
{
  Fl::add_fd(mSatSocketFd, EyeFdMonitor, this);
  //mSatSocket->SetOption(kNoBlock, 1);
}

void FTW_Shell::UninstallFdHandler()
{
  Fl::remove_fd(mSatSocketFd);
}

void FTW_Shell::PostManage(int ray_count)
{
  if (ray_count)
  {
    for (lpFl_Window_i w=mRedrawOnAnyRay.begin(); w!=mRedrawOnAnyRay.end(); ++w)
      (*w)->redraw();
  }

  Eye::PostManage(ray_count);
}

/**************************************************************************/
// OS::A_View
/**************************************************************************/

void FTW_Shell::AbsorbRay(Ray& ray)
{
  if(ray.IsBasicChange()) {
    label_shell();
    return;
  }

  /*
  if(ray.fRQN == RayNS::RQN_link_change) {
    if(wMsgRecipient->NeedsUpdate()) {
      wMsgRecipient->Update();
    }
    return;
  }
  */

  if(ray.fFID == ShellInfo::FID()) {

    switch (ray.fRQN) {

    case ShellInfo::PRQN_set_def_subshell:
      set_canvased_subshell(ray.fBetaImg);
      break;

    case ShellInfo::PRQN_add_subshell:
      spawn_subshell(ray.fBetaImg);
      break;

    case ShellInfo::PRQN_remove_subshell:
      kill_subshell(ray.fBetaImg);
      break;

    case ShellInfo::PRQN_spawn_classview:
      SpawnMTW_View(ray.fBetaImg, true);
      break;

    case ShellInfo::PRQN_spawn_metagui:
      // printf("Shell spawning metagui of %s, template %s\n",
      //    ray.fBeta->Identify().Data(), ray.fGamma->Identify().Data());
      SpawnMetaView(ray.fBetaImg, ray.fGamma);
      break;

    case ShellInfo::PRQN_resize_window: {
      int cmoh = wOutPack->h()/cell_h();
      int nmoh = mShellInfo->GetMsgOutH();
      int delta = nmoh - cmoh;
      // printf("cur=%d new=%d delta=%d\n", cmoh, nmoh, delta);
      if(delta != 0) {
	int cch = wCanvas->h()/cell_h();
	if(cch - delta < min_canvas_H) {
	  size(w(), h() + (min_canvas_H - cch + delta)*cell_h());
	}

	wCanvas ->size(w(), wCanvas->h()  - delta*cell_h());
	wOutPack->size(w(), wOutPack->h() + delta*cell_h());
	wMainPack->init_sizes();
	wMsgPack->init_sizes();
	set_size_range();
	redraw();
      }
      break;
    }

    }

  }
}

/**************************************************************************/

OS::ZGlassImg* FTW_Shell::SearchConfigEntry(const TString& name)
{
  ZGlass* res = 0;
  try {
    res = mShellInfo->FindLensByPath(name);
    if(res == 0) res = mShellInfo->GetQueen()->FindLensByPath(name);
    if(res == 0) res = mSaturn->GetFireQueen()->FindLensByPath(name);
  }
  catch(Exc_t& exc) {
    return 0;
  }
  return DemanglePtr(res);
}

/**************************************************************************/
// SubShell spawners
/**************************************************************************/

// Request creators

void FTW_Shell::SpawnSubShell(SubShellInfo* ssi, bool markup)
{
  if(markup) {
    ssi->SetName(GForm("%s [id=%d]", ssi->GetName(), ++mSubShellCount));
    ssi->SetTitle(GForm("shell: %s", mShellInfo->GetName()));
  }

  ZQueen* queen = mShellInfo->GetQueen();
  auto_ptr<ZMIR> incarnator( queen->S_IncarnateWAttach() );
  GNS::StreamLens(*incarnator, ssi);
  auto_ptr<ZMIR> att_mir( mShellInfo->S_AddSubShell(0) );
  incarnator->ChainMIR(att_mir.get());
  Send(*incarnator);
}

void FTW_Shell::RemoveSubShell(OptoStructs::ZGlassImg* img)
{

}

// View creators

FTW_SubShell* FTW_Shell::spawn_subshell(OS::ZGlassImg* img, bool show_p)
{
  static const Exc_t _eh("FTW_Shell::spawn_subshell ");

  if(mImg2SShell.find(img) != mImg2SShell.end()) {
    Message(_eh + "view of " + img->fLens->Identify() + " exists.", MT_wrn);
    return 0;
  }
  SubShellInfo* ssinfo = dynamic_cast<SubShellInfo*>(img->fLens);
  if(ssinfo == 0) {
    Message(_eh + img->fLens->Identify() + " is not a SubShellInfo.", MT_wrn);
    return 0;
  }

  TString foo_name(GForm("SubShellCreator_%s_%s",
			ssinfo->GetCtorLibset(), ssinfo->GetCtorName()));
  long* p2foo = (long*) GledNS::FindSymbol(foo_name);
  if(!p2foo) {
    Message(_eh +"can't find symbol '"+ foo_name +"'.", MT_wrn);
    return 0;
  }
  SubShellCreator_foo ssc_foo = (SubShellCreator_foo)(*p2foo);

  FTW_SubShell* fss = 0;
  try {
    fss = ssc_foo(this, img);
    mImg2SShell[img] = fss;
  }
  catch(Exc_t& exc) {
    Message(_eh + "SubShell creation failed: '" + exc + "'.", MT_wrn);
    return 0;
  }

  if(show_p) fss->GetWindow()->show();
  return fss;
}

void FTW_Shell::kill_subshell(OS::ZGlassImg* img)
{
  static const Exc_t _eh("FTW_Shell::kill_subshell ");

  hpImg2pSShell_i i = mImg2SShell.find(img);
  if(i == mImg2SShell.end()) {
    Message(_eh + "view of " + img->fLens->Identify() + " does not exist.",
	    MT_wrn);
    return;
  }
  delete i->second;
  mImg2SShell.erase(i);
}

void FTW_Shell::set_canvased_subshell(OptoStructs::ZGlassImg* img)
{
  static const Exc_t _eh("FTW_Shell::set_canvased_subshell ");

  FTW_SubShell *new_sshell = 0;
  if (img) {
    hpImg2pSShell_i i = mImg2SShell.find(img);
    if(i == mImg2SShell.end()) {
      Message(_eh + img->fLens->Identify() + " not found.", MT_wrn);
      return;
    }
    new_sshell = i->second;
  }
  if(new_sshell == mCurSubShell) return;

  hide();

  if(mCurSubShell) {
    mCurSubShell->undock();
  }

  if (new_sshell) {
    new_sshell->dock(wCanvas);
  }

  show();

  mCurSubShell = new_sshell;
}

/**************************************************************************/
// X-methods ... Shell state changers
/**************************************************************************/

void FTW_Shell::X_SetSource(FTW::Locator& loc)
{
  mSource->wLoc_Sel->set_base(loc);
  mSource->set_type(FTW::Top_Selector::T_Locator);
}

void FTW_Shell::X_SetSource(OS::ZGlassImg* img)
{
  mSource->wDir_Sel->set_img(img);
  mSource->set_type(FTW::Top_Selector::T_Direct);
}

void FTW_Shell::X_SetSink(FTW::Locator& loc)
{
  mSink->wLoc_Sel->set_base(loc);
  mSink->set_type(FTW::Top_Selector::T_Locator);
}

void FTW_Shell::X_SetSink(OS::ZGlassImg* img)
{
  mSink->wDir_Sel->set_img(img);
  mSink->set_type(FTW::Top_Selector::T_Direct);
}

void FTW_Shell::X_ExchangeSourceAndSink()
{
  // This is fooed!!!!!!!
  cout <<"Implement me ... bugger!!!\n";
  /*
  FTW::Locator ex_source = mSource->wLoc_Sel->get_locator();
  mSource->wLoc_Sel->locator_base_change(mSink->wLoc_Sel->get_locator());
  mSink->wLoc_Sel->locator_base_change(ex_source);
  */
}

/**************************************************************************/

void FTW_Shell::X_SetBeta(OS::ZGlassImg* img)
{
  auto_ptr<ZMIR> mir (mShellInfo->S_SetBeta(img->fLens));
  Send(*mir);
}

void FTW_Shell::X_SetGamma(OS::ZGlassImg* img)
{
  auto_ptr<ZMIR> mir (mShellInfo->S_SetGamma(img->fLens));
  Send(*mir);
}

/**************************************************************************/
// X-methods ... executors of graph modificators
/**************************************************************************/

void FTW_Shell::X_SetLinkOrElement(FTW::Locator& target)
{
  static const Exc_t _eh("FTW_Shell::X_SetLinkOrElement ");

  if(!mSource->has_contents())	throw(_eh + "source has no contents.");

  if (target.is_link) {
    GNS::MethodInfo* mi = target.ant->GetLinkInfo()->fSetMethod;
    auto_ptr<ZMIR> mir ( mi->MakeMir(target.get_leaf_glass()) );
    mSource->fix_MIR_beta(mir);
    Send(*mir);
  }
  else if (target.is_list_member) {
    AList* list = target.leaf->GetParent()->fImg->GetList();
    AList::ElRep elrep = target.leaf->GetElRep();
    auto_ptr<ZMIR> mir ( list->MkMir_SetElement((ZGlass*)0, elrep) );
    mSource->fix_MIR_beta(mir);
    Send(*mir);
  }
  else {
    throw(_eh + "target is neither a link nor a list element,");
  }
}

void FTW_Shell::X_ClearLinkOrElement(FTW::Locator& target)
{
  // need sink here !!!!
  static const Exc_t _eh("FTW_Shell::X_ClearLinkOrElement ");

  if (target.is_link) {
    ZMIR mir(target.get_leaf_id(), 0);
    GNS::MethodInfo* mi = target.ant->GetLinkInfo()->fSetMethod;
    mi->ImprintMir(mir);
    Send(mir);
  }
  else if (target.is_list_member) {
    AList* list = target.leaf->GetParent()->fImg->GetList();
    AList::ElRep elrep = target.leaf->GetElRep();
    auto_ptr<ZMIR> mir ( list->MkMir_SetElement((ZGlass*)0, elrep) );
    Send(*mir);
  }
  else {
    throw(_eh + "target is neither a link nor a list element,");
  }
}

/**************************************************************************/

void FTW_Shell::X_Yank(FTW::Locator& target)
{
  // Copies target to the sink.
  // !!!! The only operation with sink implemented.
  // !!!! With fixed push to sink as a list.

  static const Exc_t _eh("FTW_Shell::X_Yank ");

  if(!target.has_contents())	throw(_eh + "target has no contents");
  mSink->swallow_victim(target.get_glass());
}

/**************************************************************************/

void FTW_Shell::X_Add(FTW::Locator& target)
{
  static const Exc_t _eh("FTW_Shell::X_Add ");

  if(!target.is_list())		throw(_eh + "target is not a list");

  AList* l = (AList*) target.get_glass();
  auto_ptr<ZMIR> mir( l->MkMir_Add(0) );
  mSource->fix_MIR_beta( mir );
  Send(*mir);
}

void FTW_Shell::X_Push(FTW::Locator& target)
{
  static const Exc_t _eh("FTW_Shell::X_Push ");

  if(!target.is_list())		throw(_eh + "target is not a list");
  if(!mSource->has_contents())	throw(_eh + "source has no contents");

  AList* l = (AList*) target.get_glass();
  auto_ptr<ZMIR> mir( l->MkMir_PushBack(0) );
  mSource->fix_MIR_beta( mir );
  Send(*mir);
}

void FTW_Shell::X_Pop(FTW::Locator& target)
{
  // need sink here !!!!
  static const Exc_t _eh("FTW_Shell::X_Pop ");

  if(!target.is_list())	throw(_eh + "target is not a list");

  AList* l = (AList*) target.get_glass();
  auto_ptr<ZMIR> mir( l->MkMir_PopBack() );
  Send(*mir);
}


void FTW_Shell::X_Unshift(FTW::Locator& target)
{
  static const Exc_t _eh("FTW_Shell::X_Unshift ");

  if(!target.is_list())		throw(_eh + "target is not a list");
  if(!mSource->has_contents())	throw(_eh + "source has no contents");

  AList* l = (AList*) target.get_glass();
  auto_ptr<ZMIR> mir( l->MkMir_PushFront(0) );
  mSource->fix_MIR_beta( mir );
  Send(*mir);
}

void FTW_Shell::X_Shift(FTW::Locator& target)
{
  // need sink here !!!!
  static const Exc_t _eh("FTW_Shell::X_Shift ");

  if(!target.is_list())	throw(_eh + "target is not a list");

  AList* l = (AList*) target.get_glass();
  auto_ptr<ZMIR> mir( l->MkMir_PopFront() );
  Send(*mir);
}

/**************************************************************************/

void FTW_Shell::X_Insert(FTW::Locator& target)
{
  static const Exc_t _eh("FTW_Shell::X_Insert ");

  if(!target.is_list_member)	throw(_eh + "target is not a list member");
  if(!mSource->has_contents())	throw(_eh + "source has no contents");

  FTW_Leaf* parent_leaf = target.leaf->GetParent();
  if(parent_leaf == 0)             throw(_eh + "no parent.");
  if(!parent_leaf->fImg->IsList()) throw(_eh + "parent not a list.");

  AList*           l = (AList*) parent_leaf->fImg->fLens;
  AList::ElRep elrep = target.leaf->GetElRep();
  auto_ptr<ZMIR> mir( l->MkMir_Insert((ZGlass*)0, elrep) );
  mSource->fix_MIR_beta( mir );
  Send(*mir);
}

void FTW_Shell::X_Remove(FTW::Locator& target)
{
  // need sink here !!!!
  static const Exc_t _eh("FTW_Shell::X_Remove ");

  if(!target.is_list_member)	throw(_eh + "target is not a list member");

  FTW_Leaf* parent_leaf = target.leaf->GetParent();
  if(parent_leaf == 0)		throw(_eh + "can not remove top level object");

  AList*           l = (AList*) parent_leaf->fImg->fLens;
  AList::ElRep elrep = target.leaf->GetElRep();
  auto_ptr<ZMIR> mir( l->MkMir_Remove(elrep) );
  Send(*mir);
}

/**************************************************************************/

void FTW_Shell::Y_SendMessage(const char* msg)
{
  static const Exc_t _eh("FTW_Shell::Y_SendMessage ");

  // Need recipient FGS::LensNameBox ...
  ZMirEmittingEntity* mee = mShellInfo->GetMessageRecipient();
  if(mee) {
    auto_ptr<ZMIR> mir( mee->S_Message(msg) );
    mir->SetRecipient(mee->HostingSaturn());
    Send(*mir);
  } else {
    Message(_eh + "MessageRecipient is not set.", MT_err);
  }
}

/**************************************************************************/

void FTW_Shell::ExportToInterpreter(OS::ZGlassImg* img, const char* varname)
{
  GNS::ClassInfo* ci = img->GetCI();
  gROOT->ProcessLine(GForm("%s* %s = (%s*)%p;", ci->fName.Data(), varname,
			  ci->fName.Data(), img->fLens));
}

/**************************************************************************/

void FTW_Shell::SpawnMetaView(OS::ZGlassImg* img, ZGlass* gui)
{
  Fl_Window* w = new Fl_Window(0,0);
  MTW_MetaView* mv = new MTW_MetaView(img, this);
  w->end();
  try {
    mv->BuildByLensGraph(gui);
  }
  catch(Exc_t& exc) {
    Message(exc, MT_err);
    return;
  }
  adopt_window(w);
  w->show();
}

void FTW_Shell::SpawnMTW_View(OS::ZGlassImg* img, bool show_p)
{
  if(img->fFullMTW_View == 0) {
    Fl_Window* w = new Fl_Window(0,0);
    MTW_ClassView* cv = new MTW_ClassView(img, this);
    w->end();
    cv->BuildVerticalView();
    adopt_window(w);
    img->fFullMTW_View = cv;
    mMTW_Views.insert(img);
  }
  if(show_p) {
    img->fFullMTW_View->GetWindow()->show();
  }
}

void FTW_Shell::SpawnMTW_View(OS::ZGlassImg* img, int x, int y, float xf, float yf)
{
  SpawnMTW_View(img, false);
  Fl_Window* win = img->fFullMTW_View->GetWindow();
  int w = win->w(), h = win->h();
  x += int(xf*w);
  y += int(yf*h);
  // border
  int left=4, top=20, right=4, bot=4;
  x -= left; y -= top; w += left+right; h += top+bot;
  // fix overshoot
  if(x < 0) x = 0;
  else if(x + w > Fl::w()) x = Fl::w() - w;
  if(y < 0) y = 0;
  else if(y + h > Fl::h()) y = Fl::h() - h;
  x += left; y += top;
  win->position(x, y);
  win->show();
}

void FTW_Shell::DitchMTW_View(OS::ZGlassImg* img)
{
  if(img->fFullMTW_View != 0) {
    delete img->fFullMTW_View;
    img->fFullMTW_View = 0;
    mMTW_Views.erase(mMTW_Views.find(img));
  }
}

void FTW_Shell::RemoveMTW_Views()
{
  set<OptoStructs::ZGlassImg*>::iterator i = mMTW_Views.begin();
  while(i != mMTW_Views.end()) {
    delete (*i)->fFullMTW_View;
    (*i)->fFullMTW_View = 0;
    ++i;
  }
  mMTW_Views.clear();
}

/**************************************************************************/

MCW_View* FTW_Shell::MakeMCW_View(OS::ZGlassImg* img, GNS::MethodInfo* cmi)
{
  static const Exc_t _eh("FTW_Shell::MakeMCW_View ");

  MCW_View* mcw = new MCW_View(this);
  try {
    mcw->ParseMethodInfo(cmi);
  }
  catch(Exc_t& exc) {
    delete mcw;
    Fl_Group::current(0);
    Message(_eh + "parsing failed: " + exc, MT_err);
    return 0;
  }
  return mcw;
}

MCW_View* FTW_Shell::SpawnMCW_View(OS::ZGlassImg* img, GNS::MethodInfo* cmi,
				   bool show_p)
{
  static const Exc_t _eh("FTW_Shell::SpawnMCW_View ");

  MCW_View* mcw = MakeMCW_View(img, cmi);
  if(mcw) {
    mcw->SetABG(img->fLens, mShellInfo->GetBeta(), mShellInfo->GetGamma());
    adopt_window(mcw);
    if(show_p) {
      mcw->hotspot(mcw);
      mcw->show();
    }
  }
  return mcw;
}

/**************************************************************************/

// Local callbacks for FTW_Shell::LocatorMenu(...);

namespace
{
  // ShellVars
  //==========

  void set_source_cb(Fl_Widget* w, FTW_Shell::mir_call_data* ud) {
    ud->shell->X_SetSource(ud->get_image());
  }
  void set_sink_cb(Fl_Widget* w, FTW_Shell::mir_call_data* ud) {
    ud->shell->X_SetSink(ud->get_image());
  }

  /**************************************************************************/

  void set_beta_cb(Fl_Widget* w, FTW_Shell::mir_call_data* ud) {
    ud->shell->X_SetBeta(ud->get_image());
  }
  void set_gamma_cb(Fl_Widget* w, FTW_Shell::mir_call_data* ud) {
    ud->shell->X_SetGamma(ud->get_image());
  }

  /**************************************************************************/

  // Lens
  //=====

  void open_full_view_cb(Fl_Widget* w, FTW_Shell::mir_call_data* ud) {
    ud->shell->SpawnMTW_View(ud->get_image());
  }

  void open_nest_cb(Fl_Widget* w, FTW_Shell::mir_call_data* ud) {
    NestInfo ni("Nest");
    ni.Add(ud->get_lens());
    ud->shell->SpawnSubShell(&ni, true);
  }


  void glass_export_cb(Fl_Widget* w, FTW_Shell::mir_call_data* ud) {
    const char* var = fl_input("Varname for %s:", "foo", ud->get_lens()->GetName());
    if(var)
      ud->shell->ExportToInterpreter(ud->get_image(), var);
  }

  void spawn_mcw_cb(Fl_Widget* w, FTW_Shell::mir_call_data* ud) {
    try {
      ud->shell->SpawnMCW_View(ud->get_image(), ud->mi);
    }
    catch(Exc_t& exc) {
      ud->shell->Message(exc.Data(), FTW_Shell::MT_err);
    }
  }
}

void FTW_Shell::FillShellVarsMenu(OS::ZGlassImg* img, Fl_Menu_Button& menu,
				mir_call_data_list& mcdl, const TString& prefix)
{
    mcdl.push_back(new mir_call_data_img(img, this, 0));

    menu.add(GForm("%sSet as Source", prefix.Data()),
	     0, (Fl_Callback*)set_source_cb, mcdl.back());
    menu.add(GForm("%sSet as Sink", prefix.Data()),
		   0, (Fl_Callback*)set_sink_cb, mcdl.back(), FL_MENU_DIVIDER);

    menu.add(GForm("%sSet as Beta", prefix.Data()),
		   0, (Fl_Callback*)set_beta_cb, mcdl.back());
    menu.add(GForm("%sSet as Gamma", prefix.Data()),
		   0, (Fl_Callback*)set_gamma_cb, mcdl.back(), FL_MENU_DIVIDER);
}

void FTW_Shell::FillLensMenu(OS::ZGlassImg* img, Fl_Menu_Button& menu,
			      mir_call_data_list& mcdl, const TString& prefix)
{
  const char* p1   = "Methods";

  mcdl.push_back(new mir_call_data_img(img, this, 0));

  menu.add(GForm("%sOpen class view ...", prefix.Data()),
	   0, (Fl_Callback*)open_full_view_cb, mcdl.back());
  menu.add(GForm("%sOpen in Nest ...", prefix.Data()),
	   0, (Fl_Callback*)open_nest_cb, mcdl.back());
  menu.add(GForm("%sExport to CINT ...", prefix.Data()),
	   0, (Fl_Callback*)glass_export_cb, mcdl.back(), FL_MENU_DIVIDER);

  menu.add(GForm("%s%s", prefix.Data(), p1), 0, 0, 0, FL_SUBMENU | FL_MENU_DIVIDER);
  { // Methods ... obtained from ClassInfo
    GNS::ClassInfo* ci = img->GetCI();
    const char* pset = "Set methods";
    while(ci) {
      TString s2(GForm("%s (%d,%d)", ci->fName.Data(), ci->fFid.fLid, ci->fFid.fCid));
      const char* p2 = s2.Data();
      for(GNS::lpMethodInfo_i cmi=ci->fMethodList.begin();
	  cmi!=ci->fMethodList.end(); ++cmi)
	{
	  mcdl.push_back(new mir_call_data_img(img, this, *cmi));
	  TString& mn( (*cmi)->fName );
	  if ((mn.Length() >= 4 && mn.BeginsWith("Set") && isupper(mn[3])) ||
	      (mn.Length() >= 6 && mn.BeginsWith("Delta") && isupper(mn[5])))
	  {
	    menu.add(GForm("%s%s/%s/%s/%s (%d; %d,%d)", prefix.Data(),
			   p1, p2, pset,
			   (*cmi)->fName.Data(), (*cmi)->fMid,
			   (*cmi)->fContextArgs.size(), (*cmi)->fArgs.size()),
		     0, (Fl_Callback*)spawn_mcw_cb, mcdl.back(), 0);
	  } else {
	    menu.add(GForm("%s%s/%s/%s (%d; %d,%d)", prefix.Data(),
			   p1, p2,
			   (*cmi)->fName.Data(), (*cmi)->fMid,
			   (*cmi)->fContextArgs.size(), (*cmi)->fArgs.size()),
		     0, (Fl_Callback*)spawn_mcw_cb, mcdl.back(), 0);
	  }
	}
      ci = ci->GetParentCI();
    };

  }

}


void FTW_Shell::FullMenu(OS::ZGlassImg* img, int x, int y)
{
  Fl_Menu_Button menu(x, y, 0, 0, 0);
  menu.textsize(cell_fontsize());

  mir_call_data_list mcdl;

  FillShellVarsMenu(img, menu, mcdl, "");
  FillLensMenu(img, menu, mcdl, "");

  menu.popup();
}

void FTW_Shell::LensMenu(OS::ZGlassImg* img, int x, int y)
{
  Fl_Menu_Button menu(x, y, 0, 0, 0);
  menu.textsize(cell_fontsize());

  mir_call_data_list mcdl;

  FillLensMenu(img, menu, mcdl, "");

  menu.popup();
}

/**************************************************************************/

void FTW_Shell::Message(const char* msg, Eye::MType_e t)
{
  Fl_Color c;
  switch(t) {
  case Eye::MT_std: c = FL_WHITE;  break;
  case Eye::MT_err: c = FL_RED;    fl_beep(FL_BEEP_ERROR); break;
  case Eye::MT_wrn: c = FL_YELLOW; fl_beep(FL_BEEP_NOTIFICATION); break;
  case Eye::MT_msg: c = FL_CYAN;   fl_beep(FL_BEEP_NOTIFICATION); break;
  default:          c = FL_GREEN;  break;
  }
  wOutPack->add_line(msg, c);
}

void FTW_Shell::Message(const TString& msg, Eye::MType_e t)
{
  Message(msg.Data(), t);
}

/**************************************************************************/
/**************************************************************************/

void FTW_Shell::SourceVis(bool on_p)
{ set_vis_of_vertical_component(mSource, on_p); }

void FTW_Shell::SinkVis(bool on_p)
{ set_vis_of_vertical_component(mSink, on_p); }

/**************************************************************************/
/**************************************************************************/

int FTW_Shell::handle(int ev)
{
  if(ev == FL_SHORTCUT && Fl::event_key() == FL_Escape) {
    iconize();
    return 1;
  }

  if(ev == FL_ENTER) Fl::focus(wCanvas->children() ? wCanvas->child(0) : this);

  int ret = Fl_Window::handle(ev);
  if(ret) return ret;

  if(ev == FL_KEYBOARD) {
    // pass kbd events to swallowed nest
    if(!Fl::event_inside(wCanvas)) ret = wCanvas->handle(ev);
  }

  return ret;
}

/**************************************************************************/
/**************************************************************************/

void FTW_Shell::label_shell()
{
  mWindowLabel = GForm("shell: %s; eye: %s", mShellInfo->GetName(), GetEyeInfo()->GetName());
  label(mWindowLabel.Data());
  redraw();
}

void FTW_Shell::set_size_range()
{
  SWM_Size_Range& s = *swm_size_range;
  s.hl = min_H + min_canvas_H + wOutPack->h()/cell_h();
  int nw=cell_w(), nh=cell_h();
  size_range(nw*s.wl, nh*s.hl, nw*s.wh, nh*s.hh, nw*s.dwf, nh*s.dhf);
}

void FTW_Shell::set_vis_of_vertical_component(Fl_Widget* w, bool on_p)
{
  if(w->visible() == on_p) return;
  if(on_p) w->show(); else w->hide();
  int dh = w->h()*(on_p ? -1 : 1);
  wCanvas->resize(wCanvas->x(), wCanvas->y() - dh,
		  wCanvas->w(), wCanvas->h() + dh);
  wMainPack->init_sizes();
  redraw();
}
