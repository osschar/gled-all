// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVSiteViz_GL_RNR_H
#define AliEnViz_AEVSiteViz_GL_RNR_H

#include <Glasses/AEVSiteViz.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class AEVSiteViz_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  AEVSiteViz*	mAEVSiteViz;

public:
  AEVSiteViz_GL_Rnr(AEVSiteViz* idol) : ZNode_GL_Rnr(idol), mAEVSiteViz(idol) { _init(); }

  virtual void Draw(RnrDriver* rd);

}; // endclass AEVSiteViz_GL_Rnr

#endif
