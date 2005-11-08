// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_WGlDirectory_GL_RNR_H
#define GledCore_WGlDirectory_GL_RNR_H

#include <Glasses/WGlDirectory.h>
#include <Glasses/WGlFrameStyle.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>
#include <Rnr/GL/ZRlFont_GL_Rnr.h>

class WGlDirectory_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  WGlDirectory*       mWGlDirectory;
  RnrModStore         mFontRMS;
  RnrModStore         mFrameRMS;

  void* m_current;
  void* m_prev_page;
  void* m_next_page;
  void* m_prev;
  void* m_next;

public:
  WGlDirectory_GL_Rnr(WGlDirectory* idol) :
    ZNode_GL_Rnr(idol), mWGlDirectory(idol),
    mFontRMS(ZRlFont::FID()), mFrameRMS(WGlFrameStyle::FID())
  { _init(); }

  // virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  // virtual void PostDraw(RnrDriver* rd);

  virtual int  Handle(RnrDriver* rd, Fl_Event& ev);

}; // endclass WGlDirectory_GL_Rnr

#endif
