// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_FltkGledStuff_H
#define GledCore_FltkGledStuff_H

#include <Eye/OptoStructs.h>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Window.H>

class ZList;

class FTW_Shell;
class Fl_Menu_Button;
class Fl_Menu_Item;

// Use this one in a public part of a Fl_Widget descendant

#define FGS_LABEL_HOLDING_WEED \
protected: \
  TString _fgs_label; \
public: \
  void set_label(const char* l)   { _fgs_label = (l != 0) ? l : ""; label(_fgs_label.Data()); redraw_label(); }

#define FGS_TOOLTIP_HOLDING_WEED \
protected: \
  TString _fgs_tooltip; \
public: \
  void set_tooltip(const char* l) { _fgs_tooltip = (l != 0) ? l : ""; tooltip(_fgs_tooltip.Data()); }

// Somewhat obscure casting tool.

#define FGS_DECLARE_CAST(_var_, _src_, _typ_) \
  _typ_* _var_ = dynamic_cast<_typ_*>(_src_)


/**************************************************************************/
// FltkGledStuff namespace
/**************************************************************************/

namespace FltkGledStuff {

  template<class T>
  T grep_parent(Fl_Widget *w) {
    T ret = 0;
    while(ret == 0 && (w = w->parent())) {
      ret = dynamic_cast<T>(w);
    }
    return ret;
  }

  FTW_Shell* grep_shell(Fl_Widget *w);
  FTW_Shell* grep_shell_or_die(Fl_Widget *w, const Exc_t& _eh);

  /**************************************************************************/

  int swm_generick_width(TString& str, int cell_w, float extra);
  int swm_label_width(TString& str, int cell_w);
  int swm_string_width(TString& str, int cell_w);

  /**************************************************************************/
  // PackEntryCollapsor
  /**************************************************************************/

  class PackEntryCollapsor : public Fl_Group {
  public:
    Fl_Widget*   mColWid;
    Fl_Button*   fBut;
    Fl_Box*      fBox;

    PackEntryCollapsor(const char* t);
    virtual ~PackEntryCollapsor() {}

    int collexp(bool resize_p=true);
    static void cb_collexp(Fl_Button* w, PackEntryCollapsor* x) {x->collexp();}
  };

  /**************************************************************************/
  // LensNameBox
  /**************************************************************************/

  class LensNameBox : public OptoStructs::A_View, public Fl_Box
  {
  protected:
    TString mToName;

  public:
    FID_t	fFID;

    LensNameBox(OptoStructs::ZGlassImg* i, int x, int y, int w, int h, const char* t=0);
    virtual ~LensNameBox() {}

    virtual void AbsorbRay(Ray& ray);

    virtual void ImagePasted(OptoStructs::ZGlassImg* new_img);
    virtual void Clear() { ChangeImage(0); }

    void ChangeImage(OptoStructs::ZGlassImg* new_img);

    void auto_label();

    virtual void draw();
    virtual int  handle(int ev);

    FGS_TOOLTIP_HOLDING_WEED
  };

  /**************************************************************************/
  // LensRepNameBox
  /**************************************************************************/

  class LensRepNameBox : public LensNameBox
  {
  public:
    LensRepNameBox(OptoStructs::ZGlassImg* i, int x, int y, int w, int h, const char* t=0);
    virtual ~LensRepNameBox() {}

    virtual void ImagePasted(OptoStructs::ZGlassImg* new_img);
  };

  /**************************************************************************/
  // LinkNameBox
  /**************************************************************************/

  class LinkNameBox : public OptoStructs::ZLinkView, public LensNameBox
  {
  protected:
    TString mToName;

  public:
    LinkNameBox(OptoStructs::ZLinkDatum* ld, int x, int y, int w, int h, const char* t=0);
    virtual ~LinkNameBox() {}

    virtual void Update();

    virtual void ImagePasted(OptoStructs::ZGlassImg* new_img);
    virtual void Clear();
  };


  /**************************************************************************/
  /**************************************************************************/
  /**************************************************************************/

  /**************************************************************************/
  // MenuBox
  /**************************************************************************/

  class MenuBox : public Fl_Button {
  private:
    void _init();

  protected:
    Fl_Menu_Item*   fMenuItem;
    Fl_Menu_Button* fMenuButton;

  public:
    MenuBox(int x, int y, int w, int h, const char* t=0);
    MenuBox(Fl_Menu_Item* mi, int w, int h, const char* t=0);
    virtual ~MenuBox();

    Fl_Menu_Item* menu_item()        { return fMenuItem; }
    void menu_item(Fl_Menu_Item* mi);

    virtual int handle(int ev);
  };

  /**************************************************************************/
  // LensChoiceMenuBox
  /**************************************************************************/
  // A_View of Source.

  class LensChoiceMenuBox : public OptoStructs::A_View, public Fl_Box
  {
  protected:
    OptoStructs::ZGlassImg*  mAlphaImg;

    TString                   mSrcLinkName;
    OptoStructs::ZLinkDatum* mSrcLinkDatum;
    FID_t                    mSrcFid;
    TString                   mSrcConfigPath;

    GledNS::MethodInfo*      mMInfo;

    void   fill_menu(ZList* list, Fl_Menu_Button& menu, TString prefix);
    ZList* get_src_list(FTW_Shell* shell);

  public:
    LensChoiceMenuBox(OptoStructs::ZGlassImg* i, int x, int y, int w, int h, const char* t=0);
    virtual ~LensChoiceMenuBox() {}

    virtual void AbsorbRay(Ray& ray);

    void EmitMir(ID_t beta_id);

    void SetSrcImg(OptoStructs::ZGlassImg* i)
    { mSrcLinkDatum = 0; SetImg(i); }
    void SetSrcLinkName(const char* n)
    { mSrcLinkDatum = 0; mSrcLinkName = n ? n : ""; }
    void SetSrcFid(FID_t fid)
    { mSrcFid = fid; }

    void SetSrcConfigPath(const char* n)
    { mSrcConfigPath = n; }

    void SetMethodInfo(GledNS::MethodInfo* mi) { mMInfo = mi; }

    virtual int handle(int ev);

  };

}

#endif
