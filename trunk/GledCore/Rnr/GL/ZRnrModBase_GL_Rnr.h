// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZRnrModBase_GL_RNR_H
#define GledCore_ZRnrModBase_GL_RNR_H

#include <Glasses/ZRnrModBase.h>
#include <Rnr/GL/ZGlass_GL_Rnr.h>
#include <RnrBase/RnrDriver.h>

class ZRnrModBase_GL_Rnr : public ZGlass_GL_Rnr {
private:
  void _init();

protected:
  ZRnrModBase*	     mZRnrModBase;

  RnrDriver::RnrMod* mRnrMod;
  UInt_t             mRebuildRnrCount;

public:
  ZRnrModBase_GL_Rnr(ZRnrModBase* idol) :
    ZGlass_GL_Rnr(idol), mZRnrModBase(idol)
  { _init(); }
  virtual ~ZRnrModBase_GL_Rnr();

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass ZRnrModBase_GL_Rnr

#endif
