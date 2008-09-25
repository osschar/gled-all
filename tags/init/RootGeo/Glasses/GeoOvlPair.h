// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef RootGeo_GeoOvlPair_H
#define RootGeo_GeoOvlPair_H

#include <Glasses/ZGlass.h>
#include <Glasses/ZGeoNode.h>

class GeoOvlPair : public ZGlass {
  // 7777 RnrCtrl(0)
  MAC_RNR_FRIENDS(GeoOvlPair);
  friend class ZGeoOverlap; 
 
 private:
  void _init();

 protected:
  ZGeoNode*   mNode1;
  ZGeoNode*   mNode2;
  Bool_t      mIsExtrusion;

 public:
  GeoOvlPair(const Text_t* n="GeoOvlPair", const Text_t* t=0) : ZGlass(n,t){ _init(); }


#include "GeoOvlPair.h7"
  ClassDef(GeoOvlPair, 1);
}; // endclass GeoOvlPair

GlassIODef(GeoOvlPair);

#endif
