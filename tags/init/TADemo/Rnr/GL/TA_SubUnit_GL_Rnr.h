// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef TADemo_TA_SubUnit_GL_RNR_H
#define TADemo_TA_SubUnit_GL_RNR_H

#include <Glasses/TA_SubUnit.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class TA_SubUnit_GL_Rnr : public ZNode_GL_Rnr {
private:

protected:
  TA_SubUnit*	mTA_SubUnit;

public:
  TA_SubUnit_GL_Rnr(TA_SubUnit* idol) : ZNode_GL_Rnr(idol), mTA_SubUnit(idol) {}

  virtual void Draw(RnrDriver* rd);

}; // endclass TA_SubUnit_GL_Rnr

#endif
