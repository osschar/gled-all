// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliDet_TPCSegRnrMod_GL_RNR_H
#define AliDet_TPCSegRnrMod_GL_RNR_H

#include <Glasses/TPCSegRnrMod.h>
#include <Rnr/GL/ZRnrModBase_GL_Rnr.h>

class TPCSegRnrMod_GL_Rnr : public ZRnrModBase_GL_Rnr {
private:
  void _init();

protected:
  TPCSegRnrMod*	mTPCSegRnrMod;

public:
  TPCSegRnrMod_GL_Rnr(TPCSegRnrMod* idol) :
    ZRnrModBase_GL_Rnr(idol), mTPCSegRnrMod(idol)
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass TPCSegRnrMod_GL_Rnr

#endif
