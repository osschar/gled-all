// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_RecTrackRS_GL_RNR_H
#define Alice_RecTrackRS_GL_RNR_H

#include <Glasses/RecTrackRS.h>
#include <Rnr/GL/ZRnrModBase_GL_Rnr.h>

class RecTrackRS_GL_Rnr : public ZRnrModBase_GL_Rnr {
private:
  void _init();

protected:
  RecTrackRS*	mRecTrackRS;

public:
  RecTrackRS_GL_Rnr(RecTrackRS* idol) :
    ZRnrModBase_GL_Rnr(idol), mRecTrackRS(idol)
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass RecTrackRS_GL_Rnr

#endif
