// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_ZParticleRnrStyle_GL_RNR_H
#define Alice_ZParticleRnrStyle_GL_RNR_H

#include <Glasses/ZParticleRnrStyle.h>
#include <Rnr/GL/PRSBase_GL_Rnr.h>

class ZParticleRnrStyle_GL_Rnr : public PRSBase_GL_Rnr {
private:
  void _init();

protected:

public:
  ZParticleRnrStyle_GL_Rnr(ZParticleRnrStyle* idol) :
    PRSBase_GL_Rnr(idol)
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass ZParticleRnrStyle_GL_Rnr

#endif
