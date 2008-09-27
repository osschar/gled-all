// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_PRSBase_GL_RNR_H
#define Alice_PRSBase_GL_RNR_H

#include <Glasses/PRSBase.h>
#include <Rnr/GL/ZRnrModBase_GL_Rnr.h>

class PRSBase_GL_Rnr : public ZRnrModBase_GL_Rnr {
private:
  void _init();

protected:
  PRSBase*	mPRSBase;

public:
  PRSBase_GL_Rnr(PRSBase* idol) :
    ZRnrModBase_GL_Rnr(idol)
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass PRSBase_GL_Rnr

#endif
