// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_MCTrackRnrStyle_GL_RNR_H
#define Alice_MCTrackRnrStyle_GL_RNR_H

#include <Glasses/MCTrackRnrStyle.h>
#include <Rnr/GL/PRSBase_GL_Rnr.h>

class MCTrackRnrStyle_GL_Rnr : public PRSBase_GL_Rnr {
private:
  void _init();

protected:
  MCTrackRnrStyle* mTRS;

public:
  MCTrackRnrStyle_GL_Rnr(MCTrackRnrStyle* idol) :
    PRSBase_GL_Rnr(idol), mTRS(idol)
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass MCTrackRnrStyle_GL_Rnr

#endif
