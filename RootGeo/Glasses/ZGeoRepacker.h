// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef RootGeo_ZGeoRepacker_H
#define RootGeo_ZGeoRepacker_H

#include <Glasses/ZGlass.h>
#include <Glasses/ZGeoNode.h>

class TGeoManager;
class TGeoShape; class TGeoVolume; class TGeoNode;

#ifndef __CINT__
#include <ext/hash_set>
#endif

class ZGeoRepacker : public ZGlass
{
  MAC_RNR_FRIENDS(ZGeoRepacker);

private:
  void _init();

protected:
#ifndef __CINT__
  typedef hash_map<TObject*, int>            foo_t;
  typedef hash_map<TObject*, int>::iterator  foo_i;

#define _PAIN_(X) \
  foo_t mh ## X; \
  bool Has ## X(TObject* o) { foo_i i=mh ## X.find(o); return i!=mh ## X.end();} \
  void Put ## X(TObject* o) { mh ## X[o]++; } \
  int  Get ## X(TObject* o) { return mh ## X[o]; }

  _PAIN_(Shape);
  _PAIN_(Volume);
  //  _PAIN_(Node);

#undef _PAIN_
#endif

  TGeoManager*	  mSrcGeo;  //!
  TGeoManager*	  mNeoGeo;  //!

  ZLink<ZGeoNode> mRoot;    // X{gS} L{}
  TString         mOutFile; // X{GS} 7 Textor()

  void repack_geometry(ZGeoNode* zgnode, TGeoVolume* parent_volume);

public:
  ZGeoRepacker(const Text_t* n="ZGeoRepacker", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }

  void RepackGeometry(); // X{Ed} 7 MButt()


#include "ZGeoRepacker.h7"
  ClassDef(ZGeoRepacker, 1)
}; // endclass ZGeoRepacker


#endif
