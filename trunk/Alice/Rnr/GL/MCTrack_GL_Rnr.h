// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_MCTrack_GL_RNR_H
#define Alice_MCTrack_GL_RNR_H


#include <Glasses/MCTrack.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>
#include <RnrBase/RnrDriver.h>

class ZAliLoad;

class MCTrack_GL_Rnr : public ZNode_GL_Rnr {
  friend class MCTrackRnrStyle;
 private:
  void _init();

 protected:
  MCTrack*      mMCTrack;
  RnrModStore   mParticleRMS;

 public:
  MCTrack_GL_Rnr(MCTrack* idol) :
    ZNode_GL_Rnr(idol), mMCTrack(idol), mParticleRMS(FID_t(0,0))
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);
  virtual void Render(RnrDriver* rd);

}; // endclass MCTrack_GL_Rnr

#endif
