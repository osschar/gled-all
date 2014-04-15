// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Crawler_GL_RNR_H
#define Var1_Crawler_GL_RNR_H

#include <Glasses/Crawler.h>
#include <Rnr/GL/Extendio_GL_Rnr.h>

class Crawler_GL_Rnr : public Extendio_GL_Rnr
{
protected:
  Crawler*	mCrawler;

  Float_t       mLastLaserUpDn, mLastLaserLtRt;
  HPointF       mLaserEnd;

public:
  Crawler_GL_Rnr(Crawler* idol);
  virtual ~Crawler_GL_Rnr();

  // virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  // virtual void PostDraw(RnrDriver* rd);

  // virtual void Render(RnrDriver* rd);

}; // endclass Crawler_GL_Rnr

#endif
