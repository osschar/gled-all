// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlColorFader_GL_RNR_H
#define GledCore_ZGlColorFader_GL_RNR_H

#include <Glasses/ZGlColorFader.h>
#include <Rnr/GL/ZRnrModBase_GL_Rnr.h>

class ZGlColorFader_GL_Rnr : public ZRnrModBase_GL_Rnr {
private:
  void _init();

protected:
  ZGlColorFader* mZGlColorFader;

public:
  ZGlColorFader_GL_Rnr(ZGlColorFader* idol) :
    ZRnrModBase_GL_Rnr(idol), mZGlColorFader(idol)
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);
}; // endclass ZGlColorFader_GL_Rnr

#endif
