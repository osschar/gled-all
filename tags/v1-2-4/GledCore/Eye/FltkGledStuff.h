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

  /**************************************************************************/

  int swm_generick_width(string& str, int cell_w, float extra);
  int swm_label_width(string& str, int cell_w);
  int swm_string_width(string& str, int cell_w);

  /**************************************************************************/

  class LensNameBox : public OptoStructs::A_View, public Fl_Box {
  public:
    FID_t	fFID;

    LensNameBox(OptoStructs::ZGlassImg* i, int x, int y, int w, int h);
    ~LensNameBox();

    virtual void AbsorbRay(Ray& ray);

    void ChangeImage(OptoStructs::ZGlassImg* new_img);

    virtual void auto_label();
    virtual int  handle(int ev);

    FGS_LABEL_HOLDING_WEED
    FGS_TOOLTIP_HOLDING_WEED
  };

}

#endif
