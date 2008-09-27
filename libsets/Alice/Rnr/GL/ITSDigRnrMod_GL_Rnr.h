// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_ITSDigRnrMod_GL_RNR_H
#define Alice_ITSDigRnrMod_GL_RNR_H

#include <Glasses/ITSDigRnrMod.h>
#include <Rnr/GL/ZRnrModBase_GL_Rnr.h>

class ITSDigRnrMod_GL_Rnr : public ZRnrModBase_GL_Rnr {
private:
  void _init();

protected:
  ITSDigRnrMod*	mITSDigRnrMod;

public:
  ITSDigRnrMod_GL_Rnr(ITSDigRnrMod* idol) :
    ZRnrModBase_GL_Rnr(idol), mITSDigRnrMod(idol)
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass ITSDigRnrMod_GL_Rnr

#endif
