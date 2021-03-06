// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_FTW_Shell_H
#define GledCore_FTW_Shell_H

#include <Gled/GledTypes.h>
class SubShellInfo;
class ShellInfo;

#include <Eye/Eye.h>
#include "FTW_SubShell.h"
#include "FTW.h"
class FTW_Window;
class FTW_Leaf;
class FTW_Nest;
class MTW_ClassView;
class MTW_MetaView;
class MCW_View;

class TSocket;

#include <FL/Fl_SWM.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Pack.H>

class Fl_OutputPack;
class Fl_Menu_Button;

/**************************************************************************/

class FTW_Shell : public Eye,
		  public OptoStructs::A_View,
		  public Fl_Double_Window,
		  public Fl_SWM_Manager, public Fl_SWM_Client
{
public:
  struct mir_call_data_list;

  typedef FTW_SubShell*	(*SubShellCreator_foo)(FTW_Shell*, OptoStructs::ZGlassImg*);

private:
  void _bootstrap();
  void _bootstrap_subshells();

protected:
  ShellInfo*	mShellInfo;	 // X{g}
  TString       mWindowLabel;

  // Standalone full-class views (MTW_Views) ... transients.
  typedef hash_map<OptoStructs::ZGlassImg*, MTW_ClassView*> hpImg2pMTW_View_t;
  typedef hpImg2pMTW_View_t::iterator                       hpImg2pMTW_View_i;
  hpImg2pMTW_View_t mMTW_Views;

  // SubShells created via custom-gui interface (Pupils and Nests).
  typedef hash_map<OptoStructs::ZGlassImg*, FTW_SubShell*> hpImg2pSShell_t;
  typedef hpImg2pSShell_t::iterator                        hpImg2pSShell_i;

  hpImg2pSShell_t mImg2SShell;
  Int_t           mSubShellCount;

  FTW_SubShell* spawn_subshell(OptoStructs::ZGlassImg* img, bool show_p=true);
  void          kill_subshell(OptoStructs::ZGlassImg* img);

  void set_canvased_subshell(OptoStructs::ZGlassImg* img);

  // Windows that get redrawn on every Ray ... pupils, in principle.
  typedef list<Fl_Window*>		lpFl_Window_t;
  typedef list<Fl_Window*>::iterator	lpFl_Window_i;

  lpFl_Window_t mRedrawOnAnyRay;

  // Widgets.
  Fl_Pack*	 wMainPack;
  Fl_Pack*       wMenuPack;
  Fl_Pack*       wMsgPack;
  Fl_Group*      wCanvas;
  FTW_SubShell*  mCurSubShell;
  Fl_OutputPack* wOutPack;
  OptoStructs::ZLinkView*        wMsgRecipient;

  FTW::Source_Selector*	mSource; // X{g}
  FTW::Sink_Selector*	mSink;   // X{g}

  void label_shell();
  void set_size_range();
  void set_vis_of_vertical_component(Fl_Widget* w, bool on_p);

public:

  static FTW_Shell* Create_FTW_Shell(TSocket*, EyeInfo*, ZGlass*);

  FTW_Shell(TSocket* sock, EyeInfo* ei, const Fl_SWM_Manager* swm_copy);
  virtual ~FTW_Shell();

  // Eye
  virtual void InstallFdHandler();
  virtual void UninstallFdHandler();
  virtual void PostManage(int ray_count);
  virtual void Message(const TString& msg, InfoStream_e t=ISmessage);

  // A_View
  virtual void AbsorbRay(Ray& ray);

  // FTW_Shell

  OptoStructs::ZGlassImg* SearchConfigEntry(const TString& name);

  void SpawnSubShell(SubShellInfo* ssi, bool markup=false);
  void RemoveSubShell(OptoStructs::ZGlassImg* img);

  void X_SetSource(FTW::Locator& loc);
  void X_SetSource(OptoStructs::ZGlassImg* img);
  void X_SetSink(FTW::Locator& loc);
  void X_SetSink(OptoStructs::ZGlassImg* img);
  void X_ExchangeSourceAndSink();

  void X_SetBeta(OptoStructs::ZGlassImg* img);
  void X_SetGamma(OptoStructs::ZGlassImg* img);

  void X_SetLinkOrElement(FTW::Locator& target);
  void X_ClearLinkOrElement(FTW::Locator& target);

  void X_Yank(FTW::Locator& target);
  void X_Add(FTW::Locator& target);
  void X_Push(FTW::Locator& target);
  void X_Unshift(FTW::Locator& target);
  void X_Insert(FTW::Locator& target);
  void X_Pop(FTW::Locator& target);
  void X_Shift(FTW::Locator& target);
  void X_Remove(FTW::Locator& target);

  void Y_SendMessage(const char* msg);

  void ExportToInterpreter(OptoStructs::ZGlassImg* img, const char* varname);

  MTW_MetaView*  SpawnMetaView(OptoStructs::ZGlassImg* img, ZGlass* gui, bool show_p);
  MTW_ClassView* SpawnMTW_View(OptoStructs::ZGlassImg* img, bool manage_p, bool show_p);
  MTW_ClassView* SpawnMTW_View(OptoStructs::ZGlassImg* img, bool manage_p, bool show_p, int x, int y, float xf=0, float yf=0);
  void UnregisterMTW_View(OptoStructs::ZGlassImg* img, MTW_View* view);
  void RemoveMTW_Views();

  MCW_View* MakeMCW_View (OptoStructs::ZGlassImg* img, GledNS::MethodInfo* cmi);
  MCW_View* SpawnMCW_View(OptoStructs::ZGlassImg* img, GledNS::MethodInfo* cmi,
			  bool show_p=true);

  void RegisterROARWindow(Fl_Window* w)   { mRedrawOnAnyRay.push_back(w); }
  void UnregisterROARWindow(Fl_Window* w) { mRedrawOnAnyRay.remove(w); }

  void FillShellVarsMenu(OptoStructs::ZGlassImg* img, Fl_Menu_Button& menu,
		       mir_call_data_list& mcdl, const TString& prefix);
  void FillLensMenu(OptoStructs::ZGlassImg* img, Fl_Menu_Button& menu,
		     mir_call_data_list& mcdl, const TString& prefix);
  void FullMenu(OptoStructs::ZGlassImg* img, int x, int y);
  void LensMenu(OptoStructs::ZGlassImg* img, int x, int y);

  // --- view toggles ---

  void SourceVis(bool on_p);
  void SinkVis(bool on_p);

  // --- fltk ---

  virtual int  handle(int ev);

  /**************************************************************************/
  // mir_call_data ... temporary structures (user-data) for callbacks
  /**************************************************************************/

  struct mir_call_data
  {
    FTW_Shell* 		shell;
    GledNS::MethodInfo*	mi;

    mir_call_data(FTW_Shell* s, GledNS::MethodInfo* m) :
      shell(s), mi(m) {}
    virtual ~mir_call_data() {}

    virtual ZGlass*                  get_lens() = 0;
    virtual OptoStructs::ZGlassImg*  get_image() = 0;
    virtual GledNS::ClassInfo*       get_class_info() = 0;
  };

  struct mir_call_data_img : public mir_call_data
  {
    OptoStructs::ZGlassImg*	img;

    mir_call_data_img(OptoStructs::ZGlassImg* i, FTW_Shell* s, GledNS::MethodInfo* m) :
      mir_call_data(s, m), img(i) {}
    virtual ~mir_call_data_img() {}

    virtual ZGlass*                  get_lens()       { return img->fLens; }
    virtual OptoStructs::ZGlassImg*  get_image()      { return img; }
    virtual GledNS::ClassInfo*       get_class_info() { return img->GetCI(); }
  };

  struct mir_call_data_loc : public mir_call_data
  {
    FTW::Locator&       loc;

    mir_call_data_loc(FTW::Locator& l, FTW_Shell* s, GledNS::MethodInfo* m) :
      mir_call_data(s, m), loc(l) {}
    virtual ~mir_call_data_loc() {}

    virtual ZGlass*                  get_lens()       { return loc.get_glass(); }
    virtual OptoStructs::ZGlassImg*  get_image()      { return loc.get_image(); }
    virtual GledNS::ClassInfo*       get_class_info() { return loc.get_class_info(); }
  };

  // And a smart container

  struct mir_call_data_list : public list<mir_call_data*>
  {
    ~mir_call_data_list() { for(iterator i=begin(); i!=end(); ++i) delete *i; }
  };

#include "FTW_Shell.h7"
}; // endclass FTW_Shell

#endif
