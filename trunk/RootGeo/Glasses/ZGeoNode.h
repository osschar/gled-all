// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
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

class ZGeoNode : public ZNode
{
  // 7777 RnrCtrl(RnrBits(2,4,6,0, 0,0,0,3))
  MAC_RNR_FRIENDS(ZGeoNode);
  friend class ZGeoOvlMgr;

private:
  void _init();
  void _assert_tnode(const Exc_t& _eh, bool ggeo_fallbackp=false);

protected:
  TGeoNode	       *mTNode;     //! X{g}
  // only restored reference to TGeoNode is its name 
  TString               mTNodeName; // X{GS}          
  Int_t			mNNodes;    // X{G}   7 ValOut(-join=>1);
  Float_t		mNodeAlpha; // X{GS}  7 Value(-range=>[0,1,1,1000])
  TString               mMaterial;  // X{GS}  7 Textor(-join=>1)
  ZColor		mColor;     // X{GSP} 7 ColorButt()
  TString		mDefFile;   // X{GS} 7 Filor()

  void setup_ztrans(ZNode* zn, TGeoMatrix* gm);
  void setup_color(Float_t alpha);

  ZGeoNode* insert_node(TGeoNode* geon, ZNode* holder, const Text_t* name);
  TGeoNode* get_tnode_search_point();
  ZGeoNode* set_holder(lStr_t& node_names); 
  Bool_t    locate_tnode(ZGeoNode* zn, TGeoNode* cur_node);

public:
  ZGeoNode(const Text_t* n="ZGeoNode", const Text_t* t=0) : ZNode(n,t) { _init(); }

  void AssertUserData();    // X{E}
  void AssignGGeoTopNode(); // X{E} 7 MButt()

  // Top-level imports from gGeoManager.
  void ImportByRegExp(const Text_t* target, TRegexp filter);
  void ImportByRegExp(const Text_t* target, const Text_t* filter); // X{E} 7 MCWButt()
  void ImportUnimported(const Text_t* target="TheRest");           // X{E}

  // Daughter management.
  void ImportNodes();                      // X{Ed} 7 MButt(-join=>1)
  void Collapse();                         // X{Ed} 7 MButt()
  void ImportNodesWCollect();              // X{Ed} 7 MButt()
  void ImportNodesRec(Int_t depth=2);      // X{Ed} 7 MCWButt()

  // Persistence.
  void SaveToFile(Bool_t as_top_level, Bool_t save_links=true,
		  const Text_t* file=0);   // X{E}  7 MCWButt(-join=>1)
  void LoadFromFile(const Text_t* file=0); // X{Ed} 7 MCWButt()
  virtual void Restore();                  // X{Ed} 7 MButt()

  //--------------------------------

  // Manual Get/Set-methods
  void SetTNode(TGeoNode* n)
  { mTNode = n;  mTNodeName = n ? n->GetName() : ""; Stamp(FID()); }

  TGeoVolume* GetVolume()
  { return mTNode ? mTNode->GetVolume() : 0; }
  TObject* GetVolumeField()
  { return mTNode && mTNode->GetVolume() ? mTNode->GetVolume()->GetField() : 0; }

#include "ZGeoNode.h7"
  ClassDef(ZGeoNode, 1)
}; // endclass ZGeoNode


#endif
