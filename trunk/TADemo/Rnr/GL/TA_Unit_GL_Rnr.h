// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef TADemo_TA_Unit_GL_RNR_H
#define TADemo_TA_Unit_GL_RNR_H

#include <Glasses/TA_Unit.h>
#include <Rnr/GL/TA_SubUnit_GL_Rnr.h>

class TA_Unit_GL_Rnr : public TA_SubUnit_GL_Rnr {
private:

protected:
  TA_Unit*	mTA_Unit;

public:
  TA_Unit_GL_Rnr(TA_Unit* idol) : TA_SubUnit_GL_Rnr(idol), mTA_Unit(idol) {}

  virtual void PreDraw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass TA_Unit_GL_Rnr

#endif
