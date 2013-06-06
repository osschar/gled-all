// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_CrawlerSpiritio_GL_RNR_H
#define Var1_CrawlerSpiritio_GL_RNR_H

#include <Glasses/CrawlerSpiritio.h>
#include <Rnr/GL/Spiritio_GL_Rnr.h>

class CrawlerSpiritio_GL_Rnr : public Spiritio_GL_Rnr
{
private:
  void _init();

protected:
  CrawlerSpiritio*	mCrawlerSpiritio;

public:
  CrawlerSpiritio_GL_Rnr(CrawlerSpiritio* idol);
  virtual ~CrawlerSpiritio_GL_Rnr();

  virtual void Draw(RnrDriver* rd);

  virtual int HandleMouse(RnrDriver* rd, Fl_Event& ev);

}; // endclass CrawlerSpiritio_GL_Rnr

#endif
