// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef RootGeo_ZGeoNode_GL_RNR_H
#define RootGeo_ZGeoNode_GL_RNR_H

#include <Glasses/ZGeoNode.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>
#include <Stones/GeoUserData.h>


class ZGeoNode_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  ZGeoNode*	mZGeoNode;

public:
  ZGeoNode_GL_Rnr(ZGeoNode* idol) : ZNode_GL_Rnr(idol), mZGeoNode(idol) { _init(); }

  virtual void Draw(RnrDriver* rd);

}; // endclass ZGeoNode_GL_Rnr

#endif
