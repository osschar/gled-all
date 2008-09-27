// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_V0Track_GL_RNR_H
#define Alice_V0Track_GL_RNR_H

#include <Glasses/V0Track.h>
#include <Rnr/GL/RecTrack_GL_Rnr.h>

class V0Track_GL_Rnr : public RecTrack_GL_Rnr {
 private:
  void _init();

 protected:
  V0Track*	mV0Track;

 public:
  V0Track_GL_Rnr(V0Track* idol) :
    RecTrack_GL_Rnr(idol), mV0Track(idol)
  { _init(); }
  
  /*
    virtual void PreDraw(RnrDriver* rd);
    virtual void Draw(RnrDriver* rd);
    virtual void PostDraw(RnrDriver* rd);
  */
  virtual void Render(RnrDriver* rd);

}; // endclass V0Track_GL_Rnr

#endif
