// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef LIBSETNAME_CLASSNAME_GL_RNR_H
#define LIBSETNAME_CLASSNAME_GL_RNR_H

#include <Glasses/CLASSNAME.h>
#include <Rnr/GL/BASENAME_GL_Rnr.h>

class CLASSNAME_GL_Rnr : public BASENAME_GL_Rnr {
private:
  void _init();

protected:
  CLASSNAME*	mCLASSNAME;

public:
  CLASSNAME_GL_Rnr(CLASSNAME* idol) :
    BASENAME_GL_Rnr(idol), mCLASSNAME(idol)
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass CLASSNAME_GL_Rnr

#endif
