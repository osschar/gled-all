// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "MCW_View.h"
#include "FTW.h"
#include "FTW_Shell.h"
#include "FltkGledStuff.h"

#include <Eye/Eye.h>
#include <Stones/ZMIR.h>
#include <Glasses/SaturnInfo.h>

#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Light_Button.H>

#include <FL/Fl.H>
#include <FL/fl_draw.H>

#include <float.h>
#include <cmath>

namespace GNS = GledNS;
namespace OS  = OptoStructs;
namespace FGS = FltkGledStuff;


/**************************************************************************/
// MCW_View's inner classes
/**************************************************************************/

MCW_View::ArgBase::ArgBase(const TString& typ,  const TString& base_typ,
			   const TString& name, const TString& defval,
			   int cell_w) :
  Fl_Pack(0,0,0,1),
  _typ(typ), _base_typ(base_typ), _name(name)
{
  // Does NOT end() the fl group.

  type(FL_HORIZONTAL);

  insert_box(FGS::swm_label_width(_typ, cell_w),  _typ.Data());
  insert_box(FGS::swm_label_width(_name, cell_w), _name.Data());
}

/**************************************************************************/

void MCW_View::ArgBase::insert_box(int w, const char* l)
{
  Fl_Box* b = new Fl_Box(FL_ENGRAVED_BOX, 0, 0, w, 1, l);
  b->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
}

/**************************************************************************/

MCW_View::CtxArg::CtxArg(const TString& typ,  const TString& base_typ,
			 const TString& name, const TString& defval,
			 int cell_w) :
  ArgBase(typ, base_typ, name, defval, cell_w)
{
  FGS::LensNameBox* b = new FGS::LensNameBox(0, 0, 0, 14, 1);
  b->box(FL_ENGRAVED_BOX);
  // b->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
  b->fFID = GNS::FindClassID(base_typ);

  Fl_Button* but = new Fl_Button(0, 0, 4, 1, "Clr");
  but->callback((Fl_Callback*)s_Clr_cb, this);

  end();
}

void MCW_View::CtxArg::SetImage(OS::ZGlassImg* img)
{
  ((FGS::LensNameBox*)child(2))->ChangeImage(img);
}

OS::ZGlassImg* MCW_View::CtxArg::GetImage()
{
  return ((FGS::LensNameBox*)child(2))->fImg;
}

/**************************************************************************/
//#include <limits.h>

namespace {
  struct arg_type {
    const char*    type;
    unsigned char  weed_type;
    int		   weed_w;
    double	   min, max;
    int		   stepA, stepB;
  };

  arg_type VarArgTypes[] = {
    { "Char_t",   1, 4, -1<<7,    (1<<7)-1,    1, 1 },     // 0
    { "UChar_t",  1, 4, 0,        (1<<8)-1,    1, 1 },     // 1
    { "Short_t",  1, 6, -1<<15,   (1<<15)-1,   1, 1 },     // 2
    { "UShort_t", 1, 6, 0,        (1<<16)-1,   1, 1 },     // 3
    { "Int_t",    1, 8, -1<<31,   (1ll<<31)-1, 1, 1 },     // 4
    { "UInt_t",   1, 8, 0,        (1ll<<32)-1, 1, 1 },     // 5
    { "Long_t",   1, 8, -1<<31,   (1ll<<31)-1, 1, 1 },     // 6
    { "ULong_t",  1, 8, 0,        (1ll<<32)-1, 1, 1 },     // 7
    { "Float_t",  1, 8, -FLT_MAX, FLT_MAX,     1, 1000 },  // 8
    { "Double_t", 1, 8, -DBL_MAX, DBL_MAX,     1, 1000 },  // 9

    { "Bool_t",   2, 4,  0,0,0,0 },

    { "Text_t",   3, 12, 0,0,0,0 },

    { "TString",  4, 12, 0,0,0,0 }, // TString and string streamed the same
    { "string",   4, 12, 0,0,0,0 }, // way, defaults as "" anyway.

    { 0 }
  };

  int typ_idx(const char* t) {
    int i=0;
    while(VarArgTypes[i].type != 0) {
      if(strcmp(t, VarArgTypes[i].type) == 0)
	return i;
      ++i;
    }
    return -1;
  };
}

MCW_View::VarArg::VarArg(const TString& typ,  const TString& base_typ,
			 const TString& name, const TString& defval,
			 int cell_w) :
  ArgBase(typ, base_typ, name, defval, cell_w)
{
  static const Exc_t _eh("MCW_View::VarArg::VarArg ");

  _typ_idx = typ_idx(base_typ.Data());
  // printf("For %s got type %d\n", base_typ.Data(), _typ_idx);
  if(_typ_idx < 0)
    throw(_eh + GForm("type '%s' not supported.", base_typ.Data()));

  arg_type& at = VarArgTypes[_typ_idx];
  switch(at.weed_type)
  {
    case 1:
    {
      Fl_Value_Input* w = new Fl_Value_Input(0, 0, at.weed_w, 1);
      w->callback((Fl_Callback*)s_Change_cb, FGS::grep_parent<MCW_View*>(this));
      w->range(at.min, at.max);
      // Darned UInt_t does not work properly ...
      // printf("%s::%s -> %lf, %lf\n", base_typ.Data(), name.Data(),
      //   w->minimum(), w->maximum());
      w->step(at.stepA, at.stepB);
      if (!defval.IsNull())
      {
	double x = strtod(defval.Data(), 0);
	if (at.stepB > at.stepA && fabs(x) < (double)at.stepA / at.stepB)
	  w->step(0, 1);
	w->value(x);
      }
      // !!! Here should add range/step control widget
      insert_box(4, "Rng");
      break;
    }
    case 2:
    {
      Fl_Light_Button* w = new Fl_Light_Button(0, 0, at.weed_w, 1);
      // w->callback((Fl_Callback*)s_Change_cb, FGS::grep_parent<MCW_View*>(this));
      if(!defval.IsNull()) {
	if(defval == "true" || defval == "")
	  w->value(1);
      }
      break;
    }
    case 3:
    case 4:
    {
      Fl_Input* w = new Fl_Input(0, 0, at.weed_w, 1);
      // w->callback((Fl_Callback*)s_Change_cb, FGS::grep_parent<MCW_View*>(this));
      if(!defval.IsNull() && defval(0) == '"' && defval(defval.Length()-1) == '"') {
	TString def(defval);
	def.Remove(0, 1); def.Remove(def.Length()-1, 1);
	w->value(def.Data());
      }
      break;
    }
  }

  end();
}

void MCW_View::VarArg::StreamData(TBuffer& b)
{
  const static TString _eh("MCW_View::VarArg::StreamData ");

  arg_type& at = VarArgTypes[_typ_idx];
  switch(at.weed_type) {
  case 1:
    {
      Fl_Value_Input* w = (Fl_Value_Input*)child(2);
      switch(_typ_idx) {
      case 0: { Char_t   x = (Char_t)   w->value(); b << x; break; }
      case 1: { UChar_t  x = (UChar_t)  w->value(); b << x; break; }
      case 2: { Short_t  x = (Short_t)  w->value(); b << x; break; }
      case 3: { UShort_t x = (UShort_t) w->value(); b << x; break; }
      case 4: { Int_t    x = (Int_t)    w->value(); b << x; break; }
      case 5: { UInt_t   x = (UInt_t)   w->value(); b << x; break; }
      case 6: { Long_t   x = (Long_t)   w->value(); b << x; break; }
      case 7: { ULong_t  x = (ULong_t)  w->value(); b << x; break; }
      case 8: { Float_t  x = (Float_t)  w->value(); b << x; break; }
      case 9: { Double_t x = (Double_t) w->value(); b << x; break; }
      default:{ printf("%sunexpected valuator type.", _eh.Data()); break; }

      }
      break;
    }
  case 2:
    {
      Fl_Light_Button* w = (Fl_Light_Button*)child(2);
      Bool_t x = w->value();
      b << x;
      break;
    }
  case 3:
    {
      Fl_Input* w = (Fl_Input*)child(2);
      const Text_t* s = w->value();
      b.WriteArray(s, s ? strlen(s)+1 : 0);
      break;
    }
  case 4:
    {
      Fl_Input* w = (Fl_Input*)child(2);
      TString s(w->value());
      b << s;
      break;
    }
  default: {
    printf("%sunexpected argument type.", _eh.Data());
    break;
  }
  }
}

/**************************************************************************/
// MCW_View
/**************************************************************************/

MCW_View::MCW_View(FTW_Shell* shell) :
  Fl_Window(0,0,0),
  FTW_ShellClient(shell)
{
  // Watch out ... end() is not called ... all done in ParseMethodInfo.
  bSendOnChange = 0;
}

/**************************************************************************/

void MCW_View::ParseMethodInfo(GledNS::MethodInfo* mi) throw(Exc_t)
{
  mMInfo = mi;
  mTitle = GForm("%s::%s", mMInfo->fClassInfo->fName.Data(), mMInfo->fName.Data());
  label(mTitle.Data());

  fl_font(fl_font(), mShell->cell_fontsize()); // Need for measuring
  int cell_w = mShell->cell_w();

  // printf("Grunching %s ...\n", mTitle.Data());

  int hctx = 0, harg = 0;
  TString t, bt, n, d;

  mPack = new Fl_Pack(0,0,0,0);
  mPack->type(FL_VERTICAL);

  mCtxPack = new Fl_Pack(0,0,0,0);
  mCtxPack->type(FL_VERTICAL);

  t = "SaturnInfo"; bt = t; t += "*"; n = "<recipient>"; d = "0";
  new CtxArg(t, bt, n, d, cell_w);
  hctx++;

  t = mMInfo->fClassInfo->fName; bt = t; t += "*"; n = "<alpha>"; d = "";
  new CtxArg(t, bt, n, d, cell_w);
  hctx++;

  for(lStr_i m=mMInfo->fContextArgs.begin(); m!=mMInfo->fContextArgs.end(); ++m) {
    GNS::split_argument(*m, t, n, d);
    bt = t;
    GNS::unrefptrconst_type(bt);
    // printf("Ctx '%s' -> '%s' [%s] '%s' '%s'\n", m->Data(),
    //   t.Data(), bt.Data(), n.Data(), d.Data());

    new CtxArg(t, bt, n, d, cell_w);
    hctx++;
  }
  mCtxPack->end();

  mVarPack = new Fl_Pack(0,hctx,0,0);
  mVarPack->type(FL_VERTICAL);
  for(lStr_i m=mMInfo->fArgs.begin(); m!=mMInfo->fArgs.end(); ++m) {
    GNS::split_argument(*m, t, n, d);
    bt = t;
    GNS::unrefptrconst_type(bt);
    // printf("Var '%s' -> '%s' [%s] '%s' '%s'\n", m->Data(),
    //  t.Data(), bt.Data(), n.Data(), d.Data());

    new VarArg(t, bt, n, d, cell_w);
    harg++;
  }
  mVarPack->end();

  mPack->end();
  end();

  int wmaxs[5];
  memset(wmaxs, 0, 5*sizeof(int));
  deduce_max_widths(mCtxPack, wmaxs);
  deduce_max_widths(mVarPack, wmaxs);
  for(int i=0;i<4;++i) wmaxs[4] += wmaxs[i];
  apply_max_widths(mCtxPack, wmaxs);
  apply_max_widths(mVarPack, wmaxs);

  mCtxPack->size(wmaxs[4], hctx);	mCtxPack->resizable(0);
  mVarPack->size(wmaxs[4], harg);	mVarPack->resizable(0);
  mPack->size(wmaxs[4], hctx + harg);	mPack->resizable(0);

  {
    Fl_Group* butg = new Fl_Group(0, 0, wmaxs[4], 1);
    Fl_Button* b;
    int butw = (wmaxs[4]-6)/4;
    int x = 0;

    b = new Fl_Button(x, 0, butw+2, 1, "Send&Exit");
    b->callback((Fl_Callback*)s_SendExit_cb, this);
    x += butw+2;

    b = new Fl_Button(x, 0, butw, 1, "&Send");
    b->callback((Fl_Callback*)s_Send_cb, this);
    x += butw;

    b = new Fl_Button(x, 0, butw, 1, "E&xit");
    b->callback((Fl_Callback*)s_Exit_cb, this);
    x += butw;

    bSendOnChange = new Fl_Light_Button(x, 0, wmaxs[4] - x, 1, "Send On Change");
    bSendOnChange->tooltip("Send automatically when user changes numerical values.");

    butg->end();
    butg->resizable(butg->child(1));
    mPack->add(butg);
  }

  size(wmaxs[4], hctx + harg + 1);
  swm_size_range = new SWM_Size_Range(w(), h(), 2*w(), h());
}

void MCW_View::SetABG(ZGlass* alpha, ZGlass* beta, ZGlass* gamma)
{
  ZGlass* xx[] = { 0, alpha, beta, gamma };
  for(int c=1; c<mCtxPack->children() && c<=3; ++c) {
    CtxArg* a = dynamic_cast<CtxArg*>( mCtxPack->child(c) );
    if(a)   a->SetImage(mShell->DemanglePtr(xx[c]));
  }
}

void MCW_View::SetABG(ID_t alpha, ID_t beta, ID_t gamma)
{
  ID_t xx[] = { 0, alpha, beta, gamma };
  for(int c=1; c<mCtxPack->children() && c<=3; ++c) {
    CtxArg* a = dynamic_cast<CtxArg*>( mCtxPack->child(c) );
    if(a)   a->SetImage(mShell->DemangleID(xx[c]));
  }
}

void MCW_View::SetRecipient(SaturnInfo* si)
{
  CtxArg* a = dynamic_cast<CtxArg*>( mCtxPack->child(0) );
  if(a)   a->SetImage(mShell->DemanglePtr(si));
}

void MCW_View::SetRecipient(ID_t si)
{
  CtxArg* a = dynamic_cast<CtxArg*>( mCtxPack->child(0) );
  if(a)   a->SetImage(mShell->DemangleID(si));
}

/**************************************************************************/

void MCW_View::Send()
{
  ID_t ctx[] = { 0,0,0,0 };
  for(int i=0; i<mCtxPack->children(); ++i) {
    OS::ZGlassImg* img = ((CtxArg*)(mCtxPack->child(i)))->GetImage();
    if(img) {
      ctx[i] = img->fLens->GetSaturnID();
    }
  }
  if(ctx[1] == 0) {
    mShell->Message("MCW_View::Send attempt to emit MIR with alpha==null blocked.",
		    ISwarning);
    return;
  }

  auto_ptr<ZMIR> mir( new ZMIR(ctx[1], ctx[2], ctx[3]) );
  mMInfo->ImprintMir(*mir);

  for(int i=0; i<mVarPack->children(); ++i) {
    VarArg* va = (VarArg*)(mVarPack->child(i));
    va->StreamData(*mir);
  }

  if(ctx[0] != 0) {
    mir->SetRecipient(dynamic_cast<SaturnInfo*>(((CtxArg*)(mCtxPack->child(0)))->GetImage()->fLens));
  }

  mShell->Send(*mir);
}

void MCW_View::ChangeCallback()
{
  if(bSendOnChange->value()) Send();
}

/**************************************************************************/

void MCW_View::resize(int X, int Y, int W, int H)
{
  int dw = W - w();
  // printf("resizabluej %p; %d - %d = %d\n", swm_manager, W, w(), dw);
  Fl_Window::resize(X,Y,W,H);
  if(dw != 0 && swm_manager && !swm_manager->in_rescale_p()) {
    resize_arg_weeds(mCtxPack, dw); mCtxPack->size(mCtxPack->w() + dw, mCtxPack->h());
    resize_arg_weeds(mVarPack, dw); mVarPack->size(mVarPack->w() + dw, mVarPack->h());
     mPack->size(mPack->w() + dw, mPack->h());
    redraw();
  }
}

int MCW_View::handle(int ev)
{
  if(ev == FL_SHORTCUT && Fl::event_key() == FL_Escape && parent() == 0) {
    delete this;
    return 1;
  }
  return Fl_Window::handle(ev);
}

/**************************************************************************/

void MCW_View::deduce_max_widths(Fl_Group* g, int *wmaxs)
{
  for(int c=0; c<g->children(); ++c) {
    ArgBase* a = dynamic_cast<ArgBase*>( g->child(c) );
    if(a) {
      for(int i=0; i<4 && i<a->children(); ++i) {
	Fl_Widget* w = a->child(i);
	if(wmaxs[i] < w->w()) wmaxs[i] = w->w();
      }
    }
  }
}

void MCW_View::apply_max_widths(Fl_Group* g, int *wmaxs)
{
  for(int c=0; c<g->children(); ++c) {
    ArgBase* a = dynamic_cast<ArgBase*>( g->child(c) );
    if(a) {
      for(int i=0; i<4 && i<a->children(); ++i) {
	Fl_Widget* w = a->child(i);
	w->size(wmaxs[i], w->h());
      }
    }
  }
}

void MCW_View::resize_arg_weeds(Fl_Group* g, int dw)
{
  for(int c=0; c<g->children(); ++c) {
    ArgBase* a = dynamic_cast<ArgBase*>( g->child(c) );
    if(a) {
      Fl_Widget* w = a->child(2);
      w->size(w->w() + dw, w->h());
      a->init_sizes();
    }
  }
}
