// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef RootGeo_ZGeoNode_H
#define RootGeo_ZGeoNode_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>
#include <Stones/GeoUserData.h>


#include <TObject.h>
#include <TGeoVolume.h>
#include <TGLSceneObject.h>
#include <TRegexp.h>

class TGeoMatrix;

class ZGeoNode : public ZNode {
  // 7777 RnrCtrl("true, true, RnrBits(2,4,6,0, 0,0,0,3)")
  MAC_RNR_FRIENDS(ZGeoNode);
  friend class ZGeoOvlMgr;

private:
  void _init();

protected:
  TGeoVolume*		mVol;       // X{GS}
  Int_t			mNNodes;    // X{G}   7 ValOut(-join=>1);
  Float_t		mNodeAlpha; // X{GS}  7 Value(-range=>[0,1,1,1000])
  TString               mMat;       // X{GS}  7 Textor(-join=>1)
  ZColor		mColor;     // X{GSP} 7 ColorButt()
public:
  void ImportNodes();              // X{E} 7 MButt(-join=>1)
  void ImportNodesWCollect();      // X{E} 7 MButt()
protected:
  Bool_t	        bRnrSelf;   // X{G} 7 Bool()

  void setup_ztrans(ZNode* zn, TGeoMatrix* gm);
  void setup_color(Float_t alpha);
  ZGeoNode* InsertNode(TGeoNode* geon, ZNode* holder, Bool_t rnr, const Text_t* name);
  ZGeoNode* LocateNodeByPath(lStr_t& node_names); 
  void ClearThisList();

public:
  ZGeoNode(const Text_t* n="ZGeoNode", const Text_t* t=0) : ZNode(n,t) { _init(); }
  
  void ImportByRegExp(const Text_t* target, TRegexp filter); 
  void ImportUnimported(const Text_t* target="TheRest"); 
  void CreateFaceset(GeoUserData* userdata = 0);
 
  virtual void RnrOffForDaughters();                        // X{ED} 7 MButt(-join=>1)
  virtual void RnrOnForDaughters();                         // X{ED} 7 MButt()
  virtual void RnrOnRec();                               // X{E}  7 MButt(-join=>1)   
  virtual void RnrOffRec();                              // X{E}  7 MButt() 
  virtual void SetRnrSelf(Bool_t rnrself);                  // X{E}

#include "ZGeoNode.h7"
  ClassDef(ZGeoNode, 1)
    }; // endclass ZGeoNode

GlassIODef(ZGeoNode);

#endif
