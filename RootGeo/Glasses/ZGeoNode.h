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
#include <TGeoNode.h>
#include <TGeoManager.h>
#include <TGLSceneObject.h>
#include <TRegexp.h>

class TGeoMatrix;

class ZGeoNode : public ZNode {
  // 7777 RnrCtrl(RnrBits(2,4,6,0, 0,0,0,3))
  MAC_RNR_FRIENDS(ZGeoNode);
  friend class ZGeoOvlMgr;

 private:
  void _init();
  // Bool_t HasTNodeRef(){return mTNodeName.IsNull();}
  void _assert_tnode(const string& _eh, bool _ggeo_fallbackp=false);

 protected:
  TGeoNode	       *mTNode;     //! X{g}
  // only restored reference to TGeoNode is its name 
  TString               mTNodeName;    // X{GS}          
  Int_t			mNNodes;    // X{G}   7 ValOut(-join=>1);
  Float_t		mNodeAlpha; // X{GS}  7 Value(-range=>[0,1,1,1000])
  TString               mMat;       // X{GS}  7 Textor(-join=>1)
  ZColor		mColor;     // X{GSP} 7 ColorButt()

  TString		mDefFile;   // X{GS} 7 Filor()

 public:
  void AssignGGeoTopNode();                // X{E}  7 MButt()
  void ImportNodes();                      // X{Ed} 7 MButt(-join=>1)
  void ImportNodesWCollect();              // X{Ed} 7 MButt()

  void SaveToFile(const Text_t* file=0);   // X{E}  7 MButt()
  void LoadFromFile(const Text_t* file=0); // X{Ed} 7 MButt()
  virtual void Restore();                  // X{Ed} 7 MButt()

 protected:
  void setup_ztrans(ZNode* zn, TGeoMatrix* gm);
  void setup_color(Float_t alpha);
  TGeoNode* get_tnode_search_point();
  ZGeoNode* insert_node(TGeoNode* geon, ZNode* holder, Bool_t rnr, const Text_t* name);
  ZGeoNode* set_holder(lStr_t& node_names); 
  Bool_t locate_tnode( ZGeoNode* zn, TGeoNode* cur_node);

 public:
  ZGeoNode(const Text_t* n="ZGeoNode", const Text_t* t=0) : ZNode(n,t) { _init(); }
  // ZGeoNode(TGeoNode* n, const Text_t* t=0) : ZNode(n->GetName(),t) {  _init(); mTNode = n}
  void ImportByRegExp(const Text_t* target, TRegexp filter); 
  void ImportUnimported(const Text_t* target="TheRest"); 
  void AssertUserData();
   
  virtual void RnrOnForDaughters();                // X{ED} 7 MButt(-join=>1)
  virtual void RnrOffForDaughters();               // X{ED} 7 MButt()
  virtual void RnrOnRec();                         // X{ED} 7 MButt(-join=>1)   
  virtual void RnrOffRec();                        // X{ED} 7 MButt() 

  // Manual Get/Set-methods
  void SetTNode(TGeoNode* n)
  { mTNode = n;  mTNodeName = n ? n->GetName() : ""; Stamp(FID()); }

    TGeoVolume* GetVolume() {
    if(mTNode) return mTNode->GetVolume();
    else       return 0;
  }

  TObject* GetVolumeField() {
    if(mTNode && mTNode->GetVolume())
      return mTNode->GetVolume()->GetField();
    else
      return 0;
  }

#include "ZGeoNode.h7"
  ClassDef(ZGeoNode, 1)
    }; // endclass ZGeoNode

GlassIODef(ZGeoNode);

#endif
