// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZRnrModList_GL_RNR_H
#define GledCore_ZRnrModList_GL_RNR_H

#include <Glasses/ZRnrModList.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>

class ZRnrModList_GL_Rnr : public ZGlass_GL_Rnr {
private:
  void _init();

protected:
  ZRnrModList*	mZRnrModList;

public:
  ZRnrModList_GL_Rnr(ZRnrModList* idol) :
    ZGlass_GL_Rnr(idol), mZRnrModList(idol)
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass ZRnrModList_GL_Rnr

#endif
