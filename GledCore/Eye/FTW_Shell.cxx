// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include <Glasses/ZGod.h>
#include <Glasses/ZKing.h>
#include <Glasses/ZQueen.h>
#include <Ephra/Saturn.h>
#include <Gled/GledNS.h>

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

#include <TROOT.h>



namespace GNS  = GledNS;
namespace GVNS = GledViewNS;
namespace OS = OptoStructs;
using namespace FTW;

namespace {
  int def_W = 80;
  int def_H = 16;

  int min_W = 30;
  int max_W = 240;
  int min_H = 16;
  int max_H = 120;

  string _meth_no_cat(" method not found in catalog");
}

/**************************************************************************/

namespace {
  void msg_send_cb(Fl_Input* w, FTW_Shell* s) {
    try {
      s->Y_SendMessage(w->value());
    }
    catch(string exc) {
      s->Message(exc.c_str(), FTW_Shell::MT_err);
    }

    w->redraw();
  }
}

/**************************************************************************/

FTW_Shell::FTW_Shell(OS::ZGlassImg* img, const Fl_SWM_Manager* swm_copy) :
  OS::A_View(img),
  Fl_Window(def_W, 6 + 1 + 20 + 1 + 8 + 1 + 1),
  Fl_SWM_Manager(swm_copy), Fl_SWM_Client()
{
  // init
  mEye = fImg->fEye;
  mShellInfo = dynamic_cast<ShellInfo*>(fImg->fGlass);
  assert(mShellInfo);
  fImg->CheckInFullView(this);

  OS::ZGlassImg* nests_img = mEye->DemanglePtr(mShellInfo->GetNests());
  pNestAm = new FTW::NestAm(this, nests_img);

  // Weeds
  wMainPack = new Fl_Pack(0, 0, w(), h());
  wMainPack->type(FL_VERTICAL);

  // Fl_Box* shell_menu = new Fl_Box(FL_BORDER_BOX, 0,0,30,2,"Here be Shell menu");
  // shell_menu->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

  mNest = new FTW_Nest(this, img, 0, 0, def_W, 22);

  mSource = new FTW::Source_Selector(this, 0, 0, 6, "Source");
  mSink   = new FTW::Sink_Selector(this, 0, 0, 6, "Sink");
  
  Fl_Box* nest_pre = new Fl_Box(FTW::separator_box,0,0,1,1, "Swallowed nest:");
  nest_pre->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  nest_pre->color(FTW::separator_color);

  // Message Output

  Fl_Box* out_pre = new Fl_Box(FTW::separator_box,0,0,1,1, "Message output:");
  out_pre->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  out_pre->color(FTW::separator_color);

  wOutPack = new Fl_OutputPack(0,0,w(),8);
  //wOutPack->base_size_mod(-1);
  wOutPack->base_skip_mod(-4);
  wOutPack->bg_color((Fl_Color)0x20202000);
  wOutPack->fg_color((Fl_Color)0xf0f0f000);

  // Message Input

  Fl_Box* in_pre = new Fl_Box(FTW::separator_box,0,0,1,1, "Message input:");
  in_pre->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  in_pre->color(FTW::separator_color);

  Fl_Input* flinp = new Fl_Input(0,0,w(),1);
  flinp->callback((Fl_Callback*)msg_send_cb, this);
  flinp->when(FL_WHEN_ENTER_KEY_ALWAYS);

  wMainPack->end();

  end();

  wMainPack->insert(*mNest, out_pre); // kludge: source/sink require nest.

  wMainPack->resizable(mNest);
  resizable(wMainPack);
  swm_size_range = new SWM_Size_Range(min_W, min_H, max_W, max_H);
  adopt_window(this);

  mNest->swm_manager = this;
  mNest->SetSWM(this, false);
  FTW_Leaf* top_leaf = FTW_Leaf::Construct(mNest, 0, img, false, false);
  mNest->InsertLeaf(top_leaf);
  top_leaf->ExpandList();

  mNest->finalize_build();
  label_shell();
}

FTW_Shell::~FTW_Shell()
{
  delete pNestAm;
  if(fImg) fImg->CheckOutFullView(this);
}

/**************************************************************************/

void FTW_Shell::AbsorbRay(Ray& ray)
{
  if(ray.IsBasicChange()) {
    label_shell();
  }
}

/**************************************************************************/
// X-methods ... Shell state changers
/**************************************************************************/

void FTW_Shell::X_SetSource(FTW::Locator& loc)
{
  mSource->wLoc_Sel->set_base(loc);
  mSource->set_type(FTW::Top_Selector::T_Locator);
}

void FTW_Shell::X_SetSource(ID_t id)
{
  mSource->wDir_Sel->set_id(id);
  mSource->set_type(FTW::Top_Selector::T_Direct);
}

void FTW_Shell::X_SetSink(FTW::Locator& loc)
{
  mSink->wLoc_Sel->set_base(loc);
  mSink->set_type(FTW::Top_Selector::T_Locator);
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

void FTW_Shell::X_SetBeta(FTW::Locator& loc)
{
  static string _eh("FTW_Shell::X_SetLink ");

  if(!loc.has_contents())	throw(_eh + "locator has no contents");

  auto_ptr<ZMIR> mir (mShellInfo->S_SetBeta(loc.get_glass()));
  fImg->fEye->Send(*mir);
}

void FTW_Shell::X_SetGamma(FTW::Locator& loc)
{
  static string _eh("FTW_Shell::X_SetLink ");

  if(!loc.has_contents())	throw(_eh + "locator has no contents");

  auto_ptr<ZMIR> mir (mShellInfo->S_SetGamma(loc.get_glass()));
  fImg->fEye->Send(*mir);
}

/**************************************************************************/
// X-methods ... executors of graph modificators
/**************************************************************************/

void FTW_Shell::X_SetLink(FTW::Locator& target)
{
  static string _eh("FTW_Shell::X_SetLink ");

  if(!target.is_link)		throw(_eh + "target is not a link");
  if(!mSource->has_contents())	throw(_eh + "source has no contents");

  GNS::MethodInfo* mi = target.ant->fLinkDatum->fLinkInfo->fSetMethod;
  auto_ptr<ZMIR> mir (mSource->generate_MIR(mi, target.get_leaf_glass()));
  fImg->fEye->Send(*mir);
}

void FTW_Shell::X_ClearLink(FTW::Locator& target)
{
  // need sink here !!!!
  static string _eh("FTW_Shell::X_ClearLink ");

  if(!target.is_link) throw(_eh + "target is not a link");

  ZMIR mir(target.get_leaf_id(), 0);
  GNS::MethodInfo* mi = target.ant->fLinkDatum->fLinkInfo->fSetMethod;
  mi->ImprintMir(mir);
  mi->FixMirBits(mir, mEye->GetSaturn()->GetSaturnInfo());
  fImg->fEye->Send(mir);
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
  fImg->fEye->Send(*mir);
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
  fImg->fEye->Send(*mir);
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
  fImg->fEye->Send(*mir);
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
    fImg->fEye->Send(*mir);
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
    fImg->fEye->Send(*mir);
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
    fImg->fEye->Send(*mir);
  } else {
    throw(string(_eh + "Remove" + _meth_no_cat));
  }
}

/**************************************************************************/

void FTW_Shell::Y_SendMessage(const char* msg)
{
  static string _eh("FTW_Shell::Y_SendMessage ");

  FTW::Locator& tgt = *mNest->RefTargetLoc();
  if(!tgt.has_contents())	throw(_eh + "target has no contents");
  EyeInfo* ei = dynamic_cast<EyeInfo*>(tgt.get_glass());
  if(!ei)	throw(_eh + "target is not of class EyeInfo");

  auto_ptr<ZMIR> mir( ei->S_Message(msg) );
  mir->SetRecipient(ei->GetMaster());
  fImg->fEye->Send(*mir);
}

/**************************************************************************/

void FTW_Shell::ExportToInterpreter(OS::ZGlassImg* img, const char* varname)
{
  GNS::ClassInfo* ci = img->fClassInfo;
  gROOT->ProcessLine(GForm("%s* %s = (%s*)%p;", ci->fName.c_str(), varname,
			  ci->fName.c_str(), img->fGlass));
}

/**************************************************************************/

FTW_Nest* FTW_Shell::SpawnNest(OS::ZGlassImg* img)
{
  FTW_Nest* nest = new FTW_Nest(this, img);
  nest->swm_manager = this;
  nest->SetSWM(this, true);
  FTW_Leaf* top_leaf = FTW_Leaf::Construct(nest, 0, img, false, false);
  nest->InsertLeaf(top_leaf);
  top_leaf->ExpandList();
  nest->finalize_build();
  return nest;
}

void FTW_Shell::SpawnMTW_View(OS::ZGlassImg* img)
{
  if(img->fFullMTW_View == 0) {
    img->fFullMTW_View = MTW_View::ConstructVerticalWindow(img, this);
    adopt_window(img->fFullMTW_View);
  }
  img->fFullMTW_View->show();
}

void FTW_Shell::DitchMTW_View(OS::ZGlassImg* img)
{
  if(img->fFullMTW_View != 0) {
    delete img->fFullMTW_View;
    img->fFullMTW_View = 0;
  }
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
  mcw->show();
}

/**************************************************************************/

// Local callbacks for FTW_Shell::LocatorMenu(...);

namespace {

  // Locator based
  //==============

  void set_source_cb(Fl_Widget* w, FTW_Shell::mir_call_data_loc* ud) {
    FTW::Locator* true_loc = new FTW::Locator(ud->loc);
    ud->shell->X_SetSource(*true_loc);
  }
  void set_sink_cb(Fl_Widget* w, FTW_Shell::mir_call_data_loc* ud) {
    FTW::Locator* true_loc = new FTW::Locator(ud->loc);
    ud->shell->X_SetSink(*true_loc);
  }

  /**************************************************************************/

  void set_beta_cb(Fl_Widget* w, FTW_Shell::mir_call_data_loc* ud) {
    ud->shell->X_SetBeta(ud->loc);
  }
  void set_gamma_cb(Fl_Widget* w, FTW_Shell::mir_call_data_loc* ud) {
    ud->shell->X_SetGamma(ud->loc);
  }
 
  /**************************************************************************/

  // Image based
  //============

  void open_full_view_cb(Fl_Widget* w, FTW_Shell::mir_call_data* ud) {
    ud->shell->SpawnMTW_View(ud->get_image());
  }

  void open_nest_cb(Fl_Widget* w, FTW_Shell::mir_call_data* ud) {
    FTW_Shell* shell = ud->shell;
    ZQueen*    queen = shell->GetShellInfo()->GetQueen();

    NestInfo ni(GForm("Nest %d",   shell->GetShellInfo()->GetNests()->Size()+1),
		GForm("shell: %s", shell->GetShellInfo()->GetName()));
    ni.Add(ud->get_lens());

    GNS::ClassInfo*  ci = GNS::FindClassInfo("ZList");
    GNS::MethodInfo* mi = ci->FindMethodInfo("Add", true);
    auto_ptr<ZMIR> incarnator
      ( queen->S_IncarnateWAttach(shell->GetShellInfo()->GetNests(), 
				  0, ci->fFid.lid, ci->fFid.cid, mi->fMid)
      );
    GNS::StreamLens(*incarnator, &ni);
    shell->GetEye()->Send(*incarnator);
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

void FTW_Shell::FillLocatorMenu(FTW::Locator& loc, Fl_Menu_Button& menu,
				mir_call_data_list& mcdl, const string& prefix)
{
    mcdl.push_back(new mir_call_data_loc(loc, this, 0));

    menu.add(GForm("%sSet as Source", prefix.c_str()),
	     0, (Fl_Callback*)set_source_cb, mcdl.back());
    menu.add(GForm("%sSet as Sink", prefix.c_str()),
		   0, (Fl_Callback*)set_sink_cb, mcdl.back(), FL_MENU_DIVIDER);

    menu.add(GForm("%sSet as Beta", prefix.c_str()),
		   0, (Fl_Callback*)set_beta_cb, mcdl.back());
    menu.add(GForm("%sSet as Gamma", prefix.c_str()),
		   0, (Fl_Callback*)set_gamma_cb, mcdl.back(), FL_MENU_DIVIDER);
}

void FTW_Shell::FillImageMenu(OS::ZGlassImg* img, Fl_Menu_Button& menu,
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


void FTW_Shell::LocatorMenu(FTW::Locator& loc, int x, int y)
{
  
  if(loc.has_contents()) {
    Fl_Menu_Button menu(x, y, 0, 0, 0);
    menu.textsize(cell_fontsize());

    mir_call_data_list mcdl;

    FillLocatorMenu(loc, menu, mcdl, "");
    FillImageMenu(loc.get_image(), menu, mcdl, "");
    
    menu.popup();
  }
}

/**************************************************************************/

void FTW_Shell::Message(const char* msg, MType_e t)
{
  Fl_Color c;
  switch(t) {
  case MT_std: c = (Fl_Color)0; break;
  case MT_err: c = FL_RED;    fl_beep(FL_BEEP_ERROR); break;
  case MT_wrn: c = FL_YELLOW; fl_beep(FL_BEEP_NOTIFICATION); break;
  case MT_msg: c = FL_CYAN;   fl_beep(FL_BEEP_NOTIFICATION); break;
  }
  wOutPack->add_line(msg, c);
}

void FTW_Shell::Message(const string& msg, MType_e t)
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

  if(ev == FL_ENTER) Fl::focus(mNest);

  int ret = Fl_Window::handle(ev);
  if(ret) return ret;

  if(ev == FL_KEYBOARD) {
    // pass kbd events to swallowed nest
    if(!Fl::event_inside(mNest)) ret = mNest->handle(ev);
  }

  return ret;
}

/**************************************************************************/
/**************************************************************************/

void FTW_Shell::label_shell()
{
  label(GForm("shell: %s; eye: %s", mShellInfo->GetName(),
	                            mEye->GetEyeInfo()->GetName()) );
  redraw();
}
