// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_FltkGledStuff_H
#define GledCore_FltkGledStuff_H

#include "OptoStructs.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Window.H>

class FTW_Shell;

// Use this one in a public part of a Fl_Widget descendant

#define FGS_LABEL_HOLDING_WEED \
protected: \
  string _fgs_label; \
public: \
  void set_label(const char* l)   { _fgs_label = (l != 0) ? l : ""; label(_fgs_label.c_str()); redraw_label(); }

#define FGS_TOOLTIP_HOLDING_WEED \
protected: \
  string _fgs_tooltip; \
public: \
  void set_tooltip(const char* l) { _fgs_tooltip = (l != 0) ? l : ""; tooltip(_fgs_tooltip.c_str()); }


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
  FTW_Shell* grep_shell_or_die(Fl_Widget *w, const string& _eh);

  /**************************************************************************/

  int swm_generick_width(string& str, int cell_w, float extra);
  int swm_label_width(string& str, int cell_w);
  int swm_string_width(string& str, int cell_w);

  /**************************************************************************/
  // LensNameBox
  /**************************************************************************/

  class LensNameBox : public OptoStructs::A_View, public Fl_Box
  {
  protected:
    string mToName;

  public:
    FID_t	fFID;

    LensNameBox(OptoStructs::ZGlassImg* i, int x, int y, int w, int h, const char* t=0);

    virtual void AbsorbRay(Ray& ray);

    virtual void ImagePasted(OptoStructs::ZGlassImg* new_img);

    void ChangeImage(OptoStructs::ZGlassImg* new_img);

    void auto_label();

    virtual void draw();
    virtual int  handle(int ev);

    FGS_TOOLTIP_HOLDING_WEED
  };

  /**************************************************************************/
  // LinkNameBox
  /**************************************************************************/

  class LinkNameBox : public OptoStructs::ZLinkView, public LensNameBox
  {
  protected:
    string mToName;

  public:
    LinkNameBox(OptoStructs::ZLinkDatum* ld, int x, int y, int w, int h, const char* t=0);

    virtual void Update();

    virtual void ImagePasted(OptoStructs::ZGlassImg* new_img);
  };

}

#endif
