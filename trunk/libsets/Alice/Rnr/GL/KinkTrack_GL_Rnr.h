// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_KinkTrack_GL_RNR_H
#define Alice_KinkTrack_GL_RNR_H

#include <Glasses/KinkTrack.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>
#include <RnrBase/RnrDriver.h>


class KinkTrack_GL_Rnr : public ZNode_GL_Rnr
{
private:
  void _init();

protected:
  KinkTrack*	mKinkTrack;
  RnrModStore	mTrackRMS;

public:
  KinkTrack_GL_Rnr(KinkTrack* idol) :
    ZNode_GL_Rnr(idol), mKinkTrack(idol), mTrackRMS(FID_t(0,0))
  { _init(); }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);
  virtual void Render(RnrDriver* rd);
}; // endclass KinkTrack_GL_Rnr

#endif
