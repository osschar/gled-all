// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_FTW_Shell_H
#define GledCore_FTW_Shell_H

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
class Fl_Menu_Button;

class FTW_Shell : public OptoStructs::A_View,
		  public Fl_Window,
		  public Fl_SWM_Manager, public Fl_SWM_Client
{
public:
  enum		MType_e { MT_std=0, MT_err, MT_wrn, MT_msg };
  struct mir_call_data_list;

protected:
  Eye*		mEye;		 // X{g}
  ShellInfo*	mShellInfo;	 // X{g}

  FTW::NestAm*	 pNestAm;

  Fl_Pack*	 wMainPack;
  FTW_Nest*	 mNest;		 // X{g}
  Fl_OutputPack* wOutPack;

  FTW::Source_Selector*	mSource; // X{g}
  FTW::Sink_Selector*	mSink;   // X{g}

  void label_shell();

public:
  FTW_Shell(OptoStructs::ZGlassImg* img, const Fl_SWM_Manager* swm_copy);
  virtual ~FTW_Shell();

  virtual void AbsorbRay(Ray& ray);

  void X_SetSource(FTW::Locator& loc);
  void X_SetSource(ID_t id);
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

  void ExportToInterpreter(OptoStructs::ZGlassImg* img, const char* varname);

  FTW_Nest* SpawnNest(OptoStructs::ZGlassImg* img);
  void	    SpawnMTW_View(OptoStructs::ZGlassImg* img);
  void	    DitchMTW_View(OptoStructs::ZGlassImg* img);
  void      SpawnMCW_View(OptoStructs::ZGlassImg* img, GledNS::MethodInfo* cmi);

  void FillLocatorMenu(FTW::Locator& loc, Fl_Menu_Button& menu,
		       mir_call_data_list& mcdl, const string& prefix);
  void FillImageMenu(OptoStructs::ZGlassImg* img, Fl_Menu_Button& menu,
		     mir_call_data_list& mcdl, const string& prefix);
  void LocatorMenu(FTW::Locator& loc, int x, int y);

  void Message(const char* msg, MType_e t=MT_std);
  void Message(const string& msg, MType_e t=MT_std);

  int handle(int ev);

  /**************************************************************************/
  // mir_call_data ... temporary structures (user-data) for callbacks
  /**************************************************************************/

    struct mir_call_data {
    FTW_Shell* 		shell;
    GledNS::MethodInfo*	mi;

    mir_call_data(FTW_Shell* s, GledNS::MethodInfo* m) :
      shell(s), mi(m) {}

    virtual ZGlass*                  get_lens() = 0;
    virtual OptoStructs::ZGlassImg*  get_image() = 0;
    virtual GledNS::ClassInfo*       get_class_info() = 0;
  };

  struct mir_call_data_img : public mir_call_data {
    OptoStructs::ZGlassImg*	img;

    mir_call_data_img(OptoStructs::ZGlassImg* i, FTW_Shell* s, GledNS::MethodInfo* m) :
      mir_call_data(s, m), img(i) {}

    virtual ZGlass*                  get_lens()       { return img->fGlass; }
    virtual OptoStructs::ZGlassImg*  get_image()      { return img; }
    virtual GledNS::ClassInfo*       get_class_info() { return img->fClassInfo; }
  };

  struct mir_call_data_loc : public mir_call_data {
    FTW::Locator&       loc;

    mir_call_data_loc(FTW::Locator& l, FTW_Shell* s, GledNS::MethodInfo* m) :
      mir_call_data(s, m), loc(l) {}

    virtual ZGlass*                  get_lens()       { return loc.get_glass(); }
    virtual OptoStructs::ZGlassImg*  get_image()      { return loc.get_image(); }
    virtual GledNS::ClassInfo*       get_class_info() { return loc.get_class_info(); }
  };

  // And a smart container

  struct mir_call_data_list : public list<mir_call_data*> {
    ~mir_call_data_list() { for(iterator i=begin(); i!=end(); ++i) delete *i; }
  };

#include "FTW_Shell.h7"
}; // endclass FTW_Shell

#endif
