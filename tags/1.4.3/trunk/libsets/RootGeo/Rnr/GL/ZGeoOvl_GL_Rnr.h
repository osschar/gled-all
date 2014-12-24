// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef RootGeo_ZGeoOvl_GL_RNR_H
#define RootGeo_ZGeoOvl_GL_RNR_H

#include <Glasses/ZGeoOvl.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>
#include  "ZGeoNode_GL_Rnr.h"

class ZGeoOvl_GL_Rnr : public ZGeoNode_GL_Rnr {
private:
  void _init();

protected:
  ZGeoOvl*	mZGeoOvl;

public:
  ZGeoOvl_GL_Rnr(ZGeoOvl* idol) : ZGeoNode_GL_Rnr(idol), mZGeoOvl(idol) { _init(); }

  virtual void Draw(RnrDriver* rd);
}; // endclass ZGeoOvl_GL_Rnr

#endif
