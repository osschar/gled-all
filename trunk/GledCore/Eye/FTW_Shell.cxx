// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include <Glasses/ZQueen.h>
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
#include "MTW_View.h"
#include "Eye.h"
#include "Pupil.h"

#include "MCW_View.h"

#include <FL/Fl_OutputPack.H>

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Button.H>
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
  static const string _eh("FTW_Shell::Create_FTW_Shell ");

  GledGUI* ggui = GledGUI::theOne;
  if(ggui == 0) throw(_eh + "GledGUI not accessible.");

  ShellInfo* si = dynamic_cast<ShellInfo*>(ud_lens);
  if(si == 0) throw(_eh + "user-data is not ShellInfo.");

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

namespace {

  /**************************************************************************/
  // Configs
  /**************************************************************************/

  int def_W = 80;
  int def_H = 16;

  int min_W = 30;
  int max_W = 240;
  int min_H = 16;
  int max_H = 120;

  string _meth_no_cat(" method not found in catalog");

  /**************************************************************************/
  // GUI low-level stuff
  /**************************************************************************/

  // New menu
  //----------

  void new_menu_cb(Fl_Menu_Button* b, void* what) {
    FTW_Shell* shell = FGS::grep_parent<FTW_Shell*>(b);

    switch(int(what)) {

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

  // Message sending callback
  //--------------------------

  void msg_send_cb(Fl_Input* w, FTW_Shell* s)
  {
    try {
      s->Y_SendMessage(w->value());
    }
    catch(string exc) {
      s->Message(exc.c_str(), Eye::MT_err);
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
  Fl_Window(def_W, 2 + 6 + 1 + 20 + 1 + 8 + 1 + 1),
  Fl_SWM_Manager(swm_copy), Fl_SWM_Client()
{
  mSubShellCount = 0;
}

FTW_Shell::~FTW_Shell()
{
  if(mSatSocket) {
    Fl::remove_fd(mSatSocket->GetDescriptor());
  }
}

void FTW_Shell::_bootstrap()
{
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

    new FGS::MenuBox(s_New_Menu, 4, 2, "New");

    FGS::LensRepNameBox* self = new FGS::LensRepNameBox(fImg, 0, 0, max_W, 2);
    self->box((Fl_Boxtype)GVNS::menubar_box);
    self->color(fl_rgb_color(220,200,200));

    wMenuPack->end();
    wMenuPack->resizable(0);
  }

  mSource = new FTW::Source_Selector(this, 0, 0, 6, "Source");
  mSink   = new FTW::Sink_Selector(this, 0, 0, 6, "Sink");
  
  Fl_Box* nest_pre = new Fl_Box(FTW::separator_box,0,0,1,1, "Swallowed SubShell:");
  nest_pre->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  nest_pre->labelfont(nest_pre->labelfont() + FL_BOLD);
  nest_pre->color(fl_rgb_color(200,220,200));

  mEmptyCanvas = new Fl_Window(0, 7, def_W, 22);
  mEmptyCanvas->end();
  mCurCanvas = mEmptyCanvas;

  // Message Output

  new FGS::PackEntryCollapsor("Message output:");

  wOutPack = new Fl_OutputPack(0,0,w(),8);
  //wOutPack->base_size_mod(-1);
  wOutPack->base_skip_mod(-4);
  wOutPack->bg_color((Fl_Color)0x20202000);
  wOutPack->fg_color((Fl_Color)0xf0f0f000);

  // Message Input

  new FGS::PackEntryCollapsor("Message input:");

  Fl_Input* flinp = new Fl_Input(0,0,w(),1);
  flinp->callback((Fl_Callback*)msg_send_cb, this);
  flinp->when(FL_WHEN_ENTER_KEY_ALWAYS);

  wMainPack->end();
  end();

  wMainPack->resizable(mCurCanvas);
  resizable(wMainPack);

  swm_size_range = new SWM_Size_Range(min_W, min_H, max_W, max_H);
  adopt_window(this);

  label_shell();
}


void FTW_Shell::_bootstrap_subshells()
{
  // Default SubShell.
  if(mShellInfo->GetDefSubShell() != 0) {
    OS::ZGlassImg* dssi = DemanglePtr(mShellInfo->GetDefSubShell());
    set_canvased_subshell(dssi);
  }

  // SubShells list.
  OS::ZGlassImg* sshells = DemanglePtr(mShellInfo->GetSubShells());

  OS::lpZGlassImg_t* imgs = sshells->GetElementImgs();
  for(OS::lpZGlassImg_i i=imgs->begin(); i!=imgs->end(); ++i) {
    spawn_subshell(*i);
  }
}

/**************************************************************************/
// Eye
/**************************************************************************/

void FTW_Shell::InstallFdHandler()
{
  Fl::add_fd(mSatSocket->GetDescriptor(), EyeFdMonitor, this);
  //mSatSocket->SetOption(kNoBlock, 1);
}

void FTW_Shell::PostManage(int ray_count)
{
  if(ray_count) {
    for(lpFl_Window_i w=mRedrawOnAnyRay.begin(); w!=mRedrawOnAnyRay.end(); ++w)
      (*w)->redraw();
  }
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

  if(ray.fFID == ShellInfo::FID()) {

    switch (ray.fRQN) {

      // here missing triangulation check for resize.

    case ShellInfo::PRQN_set_def_subshell:
      set_canvased_subshell(ray.fBetaImg);
      break;

    case ShellInfo::PRQN_add_subshell:
      spawn_subshell(ray.fBetaImg);
      break;

    case ShellInfo::PRQN_remove_subshell:
      kill_subshell(ray.fBetaImg);
      break;

    }

  }
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
  GNS::ClassInfo*  ci = mShellInfo->VGlassInfo();
  GNS::MethodInfo* mi = ci->FindMethodInfo("AddSubShell", true);
  auto_ptr<ZMIR> incarnator
      ( queen->S_IncarnateWAttach(mShellInfo, 0,
				  ci->fFid.lid, ci->fFid.cid, mi->fMid)
      );
  GNS::StreamLens(*incarnator, ssi);
  Send(*incarnator);
}

void FTW_Shell::RemoveSubShell(OptoStructs::ZGlassImg* img)
{

}

// View cretors

#include <G__ci.h>

FTW_SubShell* FTW_Shell::spawn_subshell(OS::ZGlassImg* img)
{
  static const string _eh("FTW_Shell::spawn_subshell ");

  if(mImg2SShell.find(img) != mImg2SShell.end()) {
    Message(_eh + "view of " + img->fGlass->Identify() + " exists.", MT_wrn);
    return 0;
  }
  SubShellInfo* ssinfo = dynamic_cast<SubShellInfo*>(img->fGlass);
  if(ssinfo == 0) {
    Message(_eh + img->fGlass->Identify() + " is not a SubShellInfo.", MT_wrn);
    return 0;
  }
  
  string foo_name(GForm("SubShellCreator_%s_%s",
			ssinfo->GetCtorLibset(), ssinfo->GetCtorName()));
  long* p2foo = (long*) G__findsym( foo_name.c_str() );
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
  catch(string exc) {
    Message(_eh + "SubShell creation failed: '" + exc + "'.", MT_wrn);
    return 0;
  }

  fss->GetWindow()->show();
  return fss;
}

void FTW_Shell::kill_subshell(OS::ZGlassImg* img)
{
  static const string _eh("FTW_Shell::kill_subshell ");

  hpImg2pSShell_i i = mImg2SShell.find(img);
  if(i == mImg2SShell.end()) {
    Message(_eh + "view of " + img->fGlass->Identify() + " does not exist.",
	    MT_wrn);
    return;
  }
  delete i->second;
  mImg2SShell.erase(i);
}

void FTW_Shell::set_canvased_subshell(OptoStructs::ZGlassImg* img)
{
  Fl_Window* new_canvas = 0;
  if(img == 0) {
    new_canvas = mEmptyCanvas;
  } else {
    hpImg2pSShell_i i = mImg2SShell.find(img);
    FTW_SubShell* fss = (i != mImg2SShell.end()) ? i->second :
      spawn_subshell(img);
    if(fss == 0) return;
    new_canvas = fss->GetWindow();
  }
  if(new_canvas == mCurCanvas) return;

  mCurCanvas->hide();
  if(mCurCanvas != mEmptyCanvas) {
    wMainPack->remove(mCurCanvas);
  }

  new_canvas->hide();
  if(new_canvas != mEmptyCanvas) {
    Fl_SWM_Manager::remove_window(new_canvas);
    wMainPack->insert(*new_canvas, mEmptyCanvas);
  }
  new_canvas->resize(mCurCanvas->x(), mCurCanvas->y(),
		     mCurCanvas->w(), mCurCanvas->h());
  new_canvas->show();

  if(mCurCanvas != mEmptyCanvas) {
    Fl_SWM_Manager::add_window(mCurCanvas);
    mCurCanvas->free_position();
    mCurCanvas->show();
  }

  mCurCanvas = new_canvas;
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
  auto_ptr<ZMIR> mir (mShellInfo->S_SetBeta(img->fGlass));
  Send(*mir);
}

void FTW_Shell::X_SetGamma(OS::ZGlassImg* img)
{
  auto_ptr<ZMIR> mir (mShellInfo->S_SetGamma(img->fGlass));
  Send(*mir);
}

/**************************************************************************/
// X-methods ... executors of graph modificators
/**************************************************************************/

void FTW_Shell::X_SetLink(FTW::Locator& target)
{
  static string _eh("FTW_Shell::X_SetLink ");

  if(!target.is_link)		throw(_eh + "target is not a link");
  if(!mSource->has_contents())	throw(_eh + "source has no contents");

  GNS::MethodInfo* mi = target.ant->GetLinkInfo()->fSetMethod;
  auto_ptr<ZMIR> mir (mSource->generate_MIR(mi, target.get_leaf_glass()));
  Send(*mir);
}

void FTW_Shell::X_ClearLink(FTW::Locator& target)
{
  // need sink here !!!!
  static string _eh("FTW_Shell::X_ClearLink ");

  if(!target.is_link) throw(_eh + "target is not a link");

  ZMIR mir(target.get_leaf_id(), 0);
  GNS::MethodInfo* mi = target.ant->GetLinkInfo()->fSetMethod;
  mi->ImprintMir(mir);
  mi->FixMirBits(mir, GetSaturn()->GetSaturnInfo());
  Send(mir);
}

/**************************************************************************/

void FTW_Shell::X_Yank(FTW::Locator& target)
{
  // Copies target to the sink.
  // !!!! The only operation with sink implemented.
  // !!!! With fixed push to sink as a list.

  static string _eh("FTW_Shell::X_Yank ");

  if(!target.has_contents())	throw(_eh + "target has no contents");
  mSink->swallow_victim(target.get_glass());
}

/**************************************************************************/

void FTW_Shell::X_Push(FTW::Locator& target)
{
  static string _eh("FTW_Shell::X_Push ");

  if(!target.is_list())		throw(_eh + "target is not a list");
  if(!mSource->has_contents())	throw(_eh + "source has no contents");

  GNS::MethodInfo* cmi = target.get_class_info()->FindMethodInfo("Add", true);
  if(!cmi) throw(string(_eh + "Add" + _meth_no_cat));

  auto_ptr<ZMIR> mir (mSource->generate_MIR(cmi, target.get_glass()));
  Send(*mir);
}

void FTW_Shell::X_Unshift(FTW::Locator& target)
{
  static string _eh("FTW_Shell::X_Unshift ");

  if(!target.is_list())		throw(_eh + "target is not a list");
  if(!mSource->has_contents())	throw(_eh + "source has no contents");

  GNS::MethodInfo* cmi =
    target.get_class_info()->FindMethodInfo("AddFirst", true);
  if(!cmi) throw(string(_eh + "AddFirst" +  _meth_no_cat));

  auto_ptr<ZMIR> mir (mSource->generate_MIR(cmi, target.get_glass()));
  Send(*mir);
}

void FTW_Shell::X_Insert(FTW::Locator& target)
{
  static string _eh("FTW_Shell::X_Insert ");

  if(!target.is_list_member)	throw(_eh + "target is not a list member");
  if(!mSource->has_contents())	throw(_eh + "source has no contents");

  FTW_Leaf* parent_leaf = target.leaf->GetParent();
  if(parent_leaf == 0) {
    throw(string("FTW_Shell::X_Insert target not a list member"));
  }
  ZList*  l = dynamic_cast<ZList*>(parent_leaf->fImg->fGlass);
  assert(l);

  GNS::MethodInfo* cmi = parent_leaf->fImg->fClassInfo->
    FindMethodInfo("AddBefore", true);
  if(!cmi) throw(string(_eh + "AddBefore" + _meth_no_cat));
  auto_ptr<ZMIR> mir (mSource->generate_MIR(cmi, l, target.get_glass()));
  Send(*mir);
}

/**************************************************************************/

void FTW_Shell::X_Pop(FTW::Locator& target)
{
  // need sink here !!!!
  static string _eh("FTW_Shell::X_Pop ");

  if(!target.is_list())	throw(_eh + "target is not a list");

  ZList* l = dynamic_cast<ZList*>(target.get_glass());
  ZGlass* g = l->Last();
  if(g==0) return;
  auto_ptr<ZMIR> mir( new ZMIR(l->GetSaturnID(), g->GetSaturnID()) );
  GNS::MethodInfo* cmi = 
    target.get_class_info()->FindMethodInfo("RemoveLast", true);
  if(cmi) {
    cmi->ImprintMir(*mir);
    Send(*mir);
  } else {
    throw(string(_eh + "RemoveLast" + _meth_no_cat));
  }
}

void FTW_Shell::X_Shift(FTW::Locator& target)
{
  // need sink here !!!!
  static string _eh("FTW_Shell::X_Shift ");

  if(!target.is_list())	throw(_eh + "target is not a list");

  ZList* l = dynamic_cast<ZList*>(target.get_glass());
  ZGlass* g = l->First();
  if(g==0) return;
  auto_ptr<ZMIR> mir( new ZMIR(l->GetSaturnID(), g->GetSaturnID()) );
  GNS::MethodInfo* cmi = 
    target.get_class_info()->FindMethodInfo("Remove", true);
  if(cmi) {
    cmi->ImprintMir(*mir);
    Send(*mir);
  } else {
    throw(string(_eh + "Remove" + _meth_no_cat));
  }
}

void FTW_Shell::X_Remove(FTW::Locator& target)
{
  // need sink here !!!!
  static string _eh("FTW_Shell::X_Remove ");

  if(!target.is_list_member)	throw(_eh + "target is not a list member");

  FTW_Leaf* parent_leaf = target.leaf->GetParent();
  if(parent_leaf == 0)		throw(_eh + "can not remove top level object");

  FTW_Branch* parent_branch = dynamic_cast<FTW_Branch*>(parent_leaf);

  float f = parent_branch->LeafPosition(target.leaf);

  ZList*  l = dynamic_cast<ZList*>(parent_leaf->fImg->fGlass);
  auto_ptr<ZMIR> mir( new ZMIR(l->GetSaturnID(), target.get_contents()) );

  GNS::MethodInfo* cmi = parent_leaf->fImg->fClassInfo->
    FindMethodInfo((f>0.5 ? "RemoveLast" : "Remove"), true);
  if(cmi) {
    cmi->ImprintMir(*mir);
    Send(*mir);
  } else {
    throw(string(_eh + "Remove" + _meth_no_cat));
  }
}

/**************************************************************************/

void FTW_Shell::Y_SendMessage(const char* msg)
{
  static string _eh("FTW_Shell::Y_SendMessage ");

  // Need recipient FGS::LensNameBox ...
  /*
  FTW::Locator& tgt = *mNest->RefTargetLoc();
  if(!tgt.has_contents())	throw(_eh + "target has no contents");
  EyeInfo* ei = dynamic_cast<EyeInfo*>(tgt.get_glass());
  if(!ei)	throw(_eh + "target is not of class EyeInfo");

  auto_ptr<ZMIR> mir( ei->S_Message(msg) );
  mir->SetRecipient(ei->GetMaster());
  Send(*mir);
  */
}

/**************************************************************************/

void FTW_Shell::ExportToInterpreter(OS::ZGlassImg* img, const char* varname)
{
  GNS::ClassInfo* ci = img->fClassInfo;
  gROOT->ProcessLine(GForm("%s* %s = (%s*)%p;", ci->fName.c_str(), varname,
			  ci->fName.c_str(), img->fGlass));
}

/**************************************************************************/

void FTW_Shell::SpawnMTW_View(OS::ZGlassImg* img, bool show_p)
{
  if(img->fFullMTW_View == 0) {
    Fl_Window* w = new Fl_Window(0,0);
    img->fFullMTW_View = new MTW_View(img, this);
    w->end();
    img->fFullMTW_View->BuildVerticalView();
    adopt_window(w);
    mMTW_Views.insert(img);
  }
  if(show_p) {
    img->fFullMTW_View->ShowWindow();
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

void FTW_Shell::SpawnMCW_View(OS::ZGlassImg* img, GNS::MethodInfo* cmi)
{
  static string _eh("FTW_Shell::SpawnMCW_View ");

  MCW_View* mcw = new MCW_View(this);
  try {
    mcw->ParseMethodInfo(cmi);
  }
  catch(string exc) {
    delete mcw;
    Fl_Group::current(0);
    throw(_eh + "parsing failed: " + exc);
  }
  mcw->SetABG(img->fGlass, mShellInfo->GetBeta(), mShellInfo->GetGamma());
  adopt_window(mcw);
  mcw->hotspot(mcw);
  mcw->show();
}

/**************************************************************************/

// Local callbacks for FTW_Shell::LocatorMenu(...);

namespace {

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
    catch(string exc) {
      ud->shell->Message(exc.c_str(), FTW_Shell::MT_err);
    }
  }
}

void FTW_Shell::FillShellVarsMenu(OS::ZGlassImg* img, Fl_Menu_Button& menu,
				mir_call_data_list& mcdl, const string& prefix)
{
    mcdl.push_back(new mir_call_data_img(img, this, 0));

    menu.add(GForm("%sSet as Source", prefix.c_str()),
	     0, (Fl_Callback*)set_source_cb, mcdl.back());
    menu.add(GForm("%sSet as Sink", prefix.c_str()),
		   0, (Fl_Callback*)set_sink_cb, mcdl.back(), FL_MENU_DIVIDER);

    menu.add(GForm("%sSet as Beta", prefix.c_str()),
		   0, (Fl_Callback*)set_beta_cb, mcdl.back());
    menu.add(GForm("%sSet as Gamma", prefix.c_str()),
		   0, (Fl_Callback*)set_gamma_cb, mcdl.back(), FL_MENU_DIVIDER);
}

void FTW_Shell::FillLensMenu(OS::ZGlassImg* img, Fl_Menu_Button& menu,
			      mir_call_data_list& mcdl, const string& prefix)
{
  const char* p1   = "Methods";

  mcdl.push_back(new mir_call_data_img(img, this, 0));

  menu.add(GForm("%sOpen class view ...", prefix.c_str()),
	   0, (Fl_Callback*)open_full_view_cb, mcdl.back());
  menu.add(GForm("%sOpen in Nest ...", prefix.c_str()),
	   0, (Fl_Callback*)open_nest_cb, mcdl.back());
  menu.add(GForm("%sExport to CINT ...", prefix.c_str()),
	   0, (Fl_Callback*)glass_export_cb, mcdl.back(), FL_MENU_DIVIDER);

  menu.add(GForm("%s%s", prefix.c_str(), p1), 0, 0, 0, FL_SUBMENU | FL_MENU_DIVIDER);
  { // Methods ... obtained from ClassInfo
    GNS::ClassInfo* ci = img->fClassInfo;
    const char* pset = "Set methods";
    while(ci) {
      string s2(GForm("%s (%d,%d)", ci->fName.c_str(), ci->fFid.lid, ci->fFid.cid));
      const char* p2 = s2.c_str();
      for(GNS::lpMethodInfo_i cmi=ci->fMethodList.begin();
	  cmi!=ci->fMethodList.end(); ++cmi)
	{
	  mcdl.push_back(new mir_call_data_img(img, this, *cmi));
	  string& mn( (*cmi)->fName );
	  if(mn.length() >= 4 && mn.compare(0, 3, "Set") == 0 && isupper(mn[3])) {
	    menu.add(GForm("%s%s/%s/%s/%s (%d; %d,%d)", prefix.c_str(),
			   p1, p2, pset,
			   (*cmi)->fName.c_str(), (*cmi)->fMid,
			   (*cmi)->fContextArgs.size(), (*cmi)->fArgs.size()),
		     0, (Fl_Callback*)spawn_mcw_cb, mcdl.back(), 0);
	  } else {
	    menu.add(GForm("%s%s/%s/%s (%d; %d,%d)", prefix.c_str(),
			   p1, p2,
			   (*cmi)->fName.c_str(), (*cmi)->fMid,
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
  case Eye::MT_std: c = (Fl_Color)0; break;
  case Eye::MT_err: c = FL_RED;    fl_beep(FL_BEEP_ERROR); break;
  case Eye::MT_wrn: c = FL_YELLOW; fl_beep(FL_BEEP_NOTIFICATION); break;
  case Eye::MT_msg: c = FL_CYAN;   fl_beep(FL_BEEP_NOTIFICATION); break;
  }
  wOutPack->add_line(msg, c);
}

void FTW_Shell::Message(const string& msg, Eye::MType_e t)
{
  Message(msg.c_str(), t);
}

/**************************************************************************/

int FTW_Shell::handle(int ev)
{
  if(ev == FL_SHORTCUT && Fl::event_key() == FL_Escape) {
    iconize();
    return 1;
  }

  if(ev == FL_ENTER) Fl::focus(mCurCanvas);

  int ret = Fl_Window::handle(ev);
  if(ret) return ret;

  if(ev == FL_KEYBOARD) {
    // pass kbd events to swallowed nest
    if(!Fl::event_inside(mCurCanvas)) ret = mCurCanvas->handle(ev);
  }

  return ret;
}

/**************************************************************************/
/**************************************************************************/

void FTW_Shell::label_shell()
{
  label(GForm("shell: %s; eye: %s", mShellInfo->GetName(),
	                            GetEyeInfo()->GetName()) );
  redraw();
}
