// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef RootGeo_ZGeoOverlap_H
#define RootGeo_ZGeoOverlap_H

#include <Glasses/ZNode.h>
#include <Glasses/ZGeoOvl.h>
#include <Stones/ZColor.h>

#include <TObject.h>
#include <TGeoNode.h>

typedef list<TGeoNode*>                  	 lgn_t;
typedef list<TGeoNode*>::iterator                lgn_i;
typedef list<TGeoNode*>::reverse_iterator        lgn_ri;


class ZGeoOvlMgr : public ZGeoNode {

  MAC_RNR_FRIENDS(ZGeoOvlMgr);

private:
  void _init();

  // Overlap import / markup.
protected:
  ZColor		mMotherCol;        // X{GSP}
  ZColor		mOvlCol1;          // X{GSP}
  ZColor		mOvlCol2;          // X{GSP} 
  ZColor                mExtrCol;          // X{GSP} 
  ZColor                mPM_Col;           // X{GSP} 
  Float_t               mResol;            // X{GS}  7 Value(-range=>[0,100,1,1000], -join=>1)
public:
  void                  RecalculateOvl();  // X{Ed}  7 MButt()

  // Selection of displayed overlaps.
protected:
  Float_t               mMinOvl;           // X{GS}  7 Value(-range=>[0,100,1,1000], -join=>1)
  Float_t               mMaxOvl;           // X{GS}  7 Value(-range=>[0,100,1,1000])
public:
  void                  RnrOvlInterval();  // X{ED}  7 MButt()

protected:
  ZTrans      get_ztrans(TGeoMatrix* gm);
  Bool_t      locate_first_from_top(TGeoNode* cur_node, TGeoVolume* v,
				    ZGeoNode* zn, lgn_t& result);
  string      setup_absolute_matrix(TGeoNode* top_node, TGeoVolume* v,
				    ZGeoNode* zn);
  void        setup_zcolor(ZGeoOvl* ovlm);
  ZGeoOvl*    create_standalone_node(const Text_t* n, const Text_t* t,
				     TGeoNode* tn = 0);
  void        set_tnode_by_path(string path, TGeoNode* & gn);

public:
  ZGeoOvlMgr(const Text_t* n="ZGeoOvlMgr", const Text_t* t=0):ZGeoNode(n,t){ _init();}

  void         ImportOverlaps(TObjArray* lOverlaps, TGeoNode* top_node);

  virtual void Restore();    // X{Ed} 

  void         DumpOvlMgr(); //! X{E} 7 MButt()

#include "ZGeoOvlMgr.h7"
  ClassDef(ZGeoOvlMgr, 1)
}; // endclass ZGeoOvlMgr

GlassIODef(ZGeoOvlMgr);

#endif
