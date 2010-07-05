// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_TabletRnrMod_GL_RNR_H
#define Tmp1_TabletRnrMod_GL_RNR_H

#include <Glasses/TabletRnrMod.h>
#include <Rnr/GL/ZRnrModBase_GL_Rnr.h>

class TabletRnrMod_GL_Rnr : public ZRnrModBase_GL_Rnr
{
private:
  void _init();

protected:
  TabletRnrMod*	mTabletRnrMod;

public:
  TabletRnrMod_GL_Rnr(TabletRnrMod* idol);
  virtual ~TabletRnrMod_GL_Rnr();

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

  // virtual void Render(RnrDriver* rd);

}; // endclass TabletRnrMod_GL_Rnr

#endif
