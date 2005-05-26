// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_ZParticle_GL_RNR_H
#define Alice_ZParticle_GL_RNR_H


#include <Glasses/ZParticle.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>
#include <RnrBase/RnrDriver.h>

class ZAliLoad;

class ZParticle_GL_Rnr : public ZNode_GL_Rnr {
  friend class ZParticleRnrStyle;
 private:
  void _init();

 protected:
  ZParticle*	mZParticle;
  // RnrDriver::RnrMod* mParticleRM;
  RnrModStore	       mParticleRMS;

 public:
  ZParticle_GL_Rnr(ZParticle* idol) :
    ZNode_GL_Rnr(idol), mZParticle(idol), mParticleRMS(FID_t(0,0))
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);
  virtual void Render(RnrDriver* rd);

}; // endclass ZParticle_GL_Rnr

#endif
