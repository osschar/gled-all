// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_MCW_View_H
#define GledCore_MCW_View_H

#include <Gled/GledTypes.h>
#include <Gled/GledNS.h>
#include "OptoStructs.h"
#include "FTW_SubShell.h"

#include <FL/Fl_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_SWM.H>

class MCW_View : public FTW_SubShell,
		 public Fl_Window, public Fl_SWM_Client
{

 public:

  class ArgBase : public Fl_Pack
  {
  public:
    string         _typ, _base_typ, _name;

    ArgBase(string& typ, string& base_typ,
	    string& name, string& defval, int cell_w);

    virtual void StreamData(TBuffer& b) {}

    void insert_box(int w, const char* l);
  };

  class CtxArg  : public ArgBase
  {
  public:
    FID_t	_carg_fid;

    CtxArg(string& typ, string& base_typ,
	   string& name, string& defval, int cell_w);

    void SetImage(OptoStructs::ZGlassImg* img);
    OptoStructs::ZGlassImg* GetImage();

    static void s_Clr_cb(Fl_Widget* w, CtxArg* ud)
    { ud->SetImage(0); }
  };

  class VarArg  : public ArgBase
  {
  protected:
    int  _typ_idx;

  public:
    VarArg(string& typ, string& base_typ,
	   string& name, string& defval, int cell_w);
    
    virtual void StreamData(TBuffer& b);

    void create_input_weed();
  };

 protected:
  string		mTitle;

  GledNS::MethodInfo*	mMInfo;

  Fl_Pack*		mPack;
  Fl_Pack*		mCtxPack;
  Fl_Pack*		mVarPack;

  Fl_Button*		bSendOnChange;

  void deduce_max_widths(Fl_Group* g, int *wmaxs);
  void apply_max_widths(Fl_Group* g, int *wmaxs);
  void resize_arg_weeds(Fl_Group* g, int dw);

 public:
  MCW_View(FTW_Shell* shell);

  void ParseMethodInfo(GledNS::MethodInfo* mi) throw(string);
  void SetABG(ZGlass* alpha, ZGlass* beta=0, ZGlass* gamma=0);
  void SetABG(ID_t alpha, ID_t beta=0, ID_t gamma=0);

  void Send();
  void ChangeCallback();

  virtual void resize(int X, int Y, int W, int H);
  virtual int  handle(int ev);

  static void s_Send_cb(Fl_Widget* w, MCW_View* ud)     { ud->Send(); };
  static void s_Change_cb(Fl_Widget* w, MCW_View* ud)   { ud->ChangeCallback(); };
  static void s_Exit_cb(Fl_Widget* w, MCW_View* ud)     { delete ud; };
  static void s_SendExit_cb(Fl_Widget* w, MCW_View* ud) { ud->Send(); delete ud; };

  //----------------------------------------------------------------

  static void split_argument(const string& arg,
			     string& type, string& name, string& def);
  static void unrefptrconst_type(string& type);

}; // endclass MCW_View

#endif
