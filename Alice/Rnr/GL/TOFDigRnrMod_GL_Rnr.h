// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_TOFDigRnrMod_GL_RNR_H
#define Alice_TOFDigRnrMod_GL_RNR_H

#include <Glasses/TOFDigRnrMod.h>
#include <Rnr/GL/ZRnrModBase_GL_Rnr.h>

class TOFDigRnrMod_GL_Rnr : public ZRnrModBase_GL_Rnr {
private:
  void _init();

protected:
  TOFDigRnrMod*	mTOFDigRnrMod;

public:
  TOFDigRnrMod_GL_Rnr(TOFDigRnrMod* idol) :
    ZRnrModBase_GL_Rnr(idol), mTOFDigRnrMod(idol)
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass TOFDigRnrMod_GL_Rnr

#endif
