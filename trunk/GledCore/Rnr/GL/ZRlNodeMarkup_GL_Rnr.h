// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZRlNodeMarkup_GL_RNR_H
#define GledCore_ZRlNodeMarkup_GL_RNR_H

#include <Glasses/ZRlNodeMarkup.h>
#include <Rnr/GL/ZRnrModBase_GL_Rnr.h>
#include <GledView/GLTextNS.h>

class ZRlNodeMarkup_GL_Rnr : public ZRnrModBase_GL_Rnr {
private:
  void _init();

protected:
  bool            bExState;
  ZRlNodeMarkup* mZRlNodeMarkup;

public:
  ZRlNodeMarkup_GL_Rnr(ZRlNodeMarkup* idol) :
    ZRnrModBase_GL_Rnr(idol), mZRlNodeMarkup(idol) { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass ZRlNodeMarkup_GL_Rnr

#endif
