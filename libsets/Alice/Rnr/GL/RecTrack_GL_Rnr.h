// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_RecTrack_GL_RNR_H
#define Alice_RecTrack_GL_RNR_H

#include <Glasses/RecTrack.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>
#include <RnrBase/RnrDriver.h>

class RecTrack_GL_Rnr : public ZNode_GL_Rnr
{
private:
  void _init();

protected:
  RecTrack*	mRecTrack;
  RnrModStore	mTrackRMS;

public:
  RecTrack_GL_Rnr(RecTrack* idol) :
    ZNode_GL_Rnr(idol), mRecTrack(idol), mTrackRMS(FID_t(0,0))
  { _init(); }

  virtual void Draw(RnrDriver* rd);
  virtual void Render(RnrDriver* rd);
}; // endclass RecTrack_GL_Rnr

#endif
