// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZNodeListLink_GL_RNR_H
#define GledCore_ZNodeListLink_GL_RNR_H

#include <Glasses/ZNodeListLink.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>
#include <Rnr/GL/ZRlFont_GL_Rnr.h>

class ZNodeListLink_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  ZNodeListLink*      mZNodeListLink;
  RnrModStore         mFontRMS;

public:
  ZNodeListLink_GL_Rnr(ZNodeListLink* idol) :
    ZNode_GL_Rnr(idol), mZNodeListLink(idol), mFontRMS(ZRlFont::FID())
  { _init(); }

  // virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  // virtual void PostDraw(RnrDriver* rd);

  virtual int  Handle(RnrDriver* rd, Fl_Event& ev);

}; // endclass ZNodeListLink_GL_Rnr

#endif
