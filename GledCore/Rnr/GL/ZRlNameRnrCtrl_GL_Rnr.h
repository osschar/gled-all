// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZRlNameRnrCtrl_GL_RNR_H
#define GledCore_ZRlNameRnrCtrl_GL_RNR_H

#include <Glasses/ZRlNameRnrCtrl.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>
#include <GledView/GLTextNS.h>

class ZRlNameRnrCtrl_GL_Rnr : public ZGlass_GL_Rnr {
private:
  void _init();

protected:
  bool            bExState;
  ZRlNameRnrCtrl* mZRlNameRnrCtrl;

public:
  ZRlNameRnrCtrl_GL_Rnr(ZRlNameRnrCtrl* idol) :
    ZGlass_GL_Rnr(idol), mZRlNameRnrCtrl(idol) { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass ZRlNameRnrCtrl_GL_Rnr

#endif
