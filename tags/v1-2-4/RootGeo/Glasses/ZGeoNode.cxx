// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZGeoNode
//
//

#include "ZGeoNode.h"
#include "ZGeoNode.c7"
// #include <Glasses/ZNode.cxx>
#include <Glasses/ZQueen.h>

#include <TBuffer3D.h>
#include <TGeoNode.h>
#include <TColor.h>

typedef list<ZNode*>           lpZNode_t;
typedef list<ZNode*>::iterator lpZNode_i;


ClassImp(ZGeoNode)

/**************************************************************************/
  void ZGeoNode::_init()
{
  // !!!! Set all links to 0 !!!! 
  mVol       = 0;
  mNNodes    = 0;
  mNodeAlpha = 1;
  bRnrSelf = true;

}


/**************************************************************************/
void ZGeoNode::SetRnrSelf(Bool_t rnrself) 
{
  bRnrSelf = rnrself;
  Stamp(FID());
}

/**************************************************************************/
void ZGeoNode::RnrOnForDaughters()
{
  lpZNode_t dts; 
  ZNode::CopyByGlass<ZNode*>(dts);
  
  for(lpZNode_i i=dts.begin(); i!=dts.end(); ++i) {
    ZGeoNode* d = (ZGeoNode*)(*i);
    d->WriteLock();
    d->SetRnrSelf(true);
    d->WriteUnlock();
  }
}

/***********************************************************************/
void ZGeoNode::RnrOffForDaughters()
{ 
  lpZNode_t dts; 
  ZNode::CopyByGlass<ZNode*>(dts);
  
  for(lpZNode_i i=dts.begin(); i!=dts.end(); ++i) {
    ZGeoNode* d = (ZGeoNode*)(*i);
    d->WriteLock();
    d->SetRnrSelf(false);
    d->WriteUnlock();
  }
}

/***********************************************************************/
void ZGeoNode::RnrOnRec()
{ 
  lpZNode_t dts; 
  ZNode::CopyByGlass<ZNode*>(dts);
  for(lpZNode_i i=dts.begin(); i!=dts.end(); ++i) {
    ZGeoNode* d = (ZGeoNode*)(*i);
    d->RnrOnRec();
  }

  SetRnrSelf(true);   
}

/***********************************************************************/
void ZGeoNode::RnrOffRec()
{ 
  lpZNode_t dts; 
  ZNode::CopyByGlass<ZNode*>(dts);
  for(lpZNode_i i=dts.begin(); i!=dts.end(); ++i) {
    ZGeoNode* d = (ZGeoNode*)(*i);
    d->RnrOffRec();
  }
  SetRnrSelf(false);   
}

/**************************************************************************/
void ZGeoNode::setup_ztrans(ZNode* zn, TGeoMatrix* gm)
{
  const Double_t* rm = gm->GetRotationMatrix();
  const Double_t* tv = gm->GetTranslation();
  ZTrans t;
  t(1,1) = rm[0]; t(1,2) = rm[1]; t(1,3) = rm[2];
  t(2,1) = rm[3]; t(2,2) = rm[4]; t(2,3) = rm[5];
  t(3,1) = rm[6]; t(3,2) = rm[7]; t(3,3) = rm[8];
  t(1,4) = tv[0]; t(2,4) = tv[1]; t(3,4) = tv[2];
  zn->SetTrans(t);
}

/************************************************************************/
void ZGeoNode::setup_color(Float_t alpha)
{
  Int_t ci = ((mVol->GetLineColor() % 8) - 1) * 4;
  if(ci < 0) ci = 0;
  TColor* c = gROOT->GetColor(ci);
  if (c) {
    SetColor(c->GetRed(), c->GetGreen(), c->GetBlue(), alpha);
  } else {
    SetColor(1, 0, 0, alpha);
  }
}

/**************************************************************************/
ZGeoNode* ZGeoNode::InsertNode(TGeoNode* geon, ZNode* holder, 
			       Bool_t rnr, const Text_t* title)
{
  TGeoVolume* v = geon->GetVolume();
  ZGeoNode *nn = new ZGeoNode(geon->GetName(), title);
    

  setup_ztrans(nn, geon->GetMatrix());
  nn->SetVol(v);
  string m = v->GetMaterial()->GetName();
  int j = m.find_first_of("$");
  m = m.substr(0,j);
  nn->SetMat(m.c_str());
  nn->mNNodes = v && v->GetNodes() ? v->GetNodes()->GetSize() : 0;
  mQueen->CheckIn(nn);
  holder->Add(nn);
  nn->bRnrSelf = rnr;
  
  GeoUserData* userdata = new GeoUserData(true, 0);
  nn->CreateFaceset(userdata);
  nn->setup_color(mNodeAlpha);

  // printf("%-16s %-6s  [%p]\n", geon->GetName(), title,  v);
  return nn;
}

/**************************************************************************/
void ZGeoNode::ImportNodes()
{
  static const string _eh("ZGeoNode::ImportNodes ");

  if(mVol == 0)
    throw(_eh + "volume not set.");

  ClearThisList();

  TIter next_node(mVol->GetNodes());
  TGeoNode* geon;
  while(geon = (TGeoNode*)next_node()) {
    const char* vname = "<no-vol>";
    const char* sname = "<no-shp>";

    TGeoVolume* v = geon->GetVolume();
    if(v) {      
      vname = v->GetName();
      TGeoShape* s = v->GetShape();
      if(s) {
	sname = s->GetName();
      }
    }

    InsertNode(geon, this, true,  GForm("%s::%s", vname, sname));
  }
}



/**************************************************************************/
void ZGeoNode::ImportByRegExp(const Text_t* target, TRegexp filter)
{
  static const string _eh("ZGeoNode::ImportByRegExp ");

  // split target into list of node names
  lStr_t node_names;
  GledNS::split_string(target, node_names, '/');
  ZGeoNode* holder = LocateNodeByPath(node_names);

  TIter next_node(mVol->GetNodes());
  TGeoNode* geon;

  while(geon = (TGeoNode*)next_node()) {
    TString vname = "<no-vol>";
    TString sname = "<no-shp>";

    TGeoVolume* v = geon->GetVolume();
    if(v) {      
      vname = v->GetName();
      
      // add only nodes with volume names maching filter
      if (!vname.Contains(filter)){
	continue;
      }
      
      TGeoShape* s = v->GetShape();
      if(s) {
	sname = s->GetName();
      }
    }

    InsertNode(geon, holder, false,  GForm("%s::%s", vname.Data(), sname.Data()));
  }
}

/**************************************************************************/
void ZGeoNode::ImportNodesWCollect() 
{
  static const string _eh("ZGeoNode::ImportNodesWCollect ");
  if(mVol == 0)
    throw(_eh + "volume not set.");
  
  ClearThisList();
  map<string, ZNode*> nmap;
  TIter next_node(mVol->GetNodes());
  TGeoNode* geon;
  while(geon = (TGeoNode*)next_node()) {
    const char* vname = "<no-vol>";
    const char* sname = "<no-shp>";

    TGeoVolume* v = geon->GetVolume();
    if(v) {      
      vname = v->GetName();
      TGeoShape* s = v->GetShape();
      if(s) {
	sname = s->GetName();
      }
    }

    ZNode* holder = 0;
    map<string, ZNode*>::iterator i = nmap.find(vname);
    if(i == nmap.end()) {
      ZNode* n = new ZNode(vname);
      mQueen->CheckIn(n);
      Add(n);
      nmap[vname] = n;
      holder = n;
    } else {
      holder = i->second;
    }
    
    InsertNode(geon, holder, false,  GForm("%s::%s", vname, sname));
  }
}

/*************************************************************/
void ZGeoNode::ImportUnimported(const Text_t* target)
{  
  static const string _eh("ZGeoNode::ImportUnimported ");

  if(mVol == 0)
    throw(_eh + "volume not set.");
   
  ZGeoNode* holder = dynamic_cast<ZGeoNode*>(GetElementByName(target));
  if ( holder == 0 ){
    // printf("create holder node %s \n", target);
    holder = new  ZGeoNode(target);
    mQueen->CheckIn(holder);
    Add(holder);
  }
 
  TIter next_node(mVol->GetNodes());
  TGeoNode* geon;

  while(geon = (TGeoNode*)next_node()) {
    const char* vname = "<no-vol>";
    const char* sname = "<no-shp>";

    TGeoVolume* v = geon->GetVolume();
    if(v) {      
      vname = v->GetName();
      TGeoShape* s = v->GetShape();
      if(s) {
	sname = s->GetName();
      }

      GeoUserData* ud = dynamic_cast<GeoUserData*> (v->GetField());     
      if (ud == 0) {
        InsertNode(geon, holder, false, GForm("%s::%s", vname, sname));
      }
      
      if (ud && ((ud->bIsImported) == false)) {
        InsertNode(geon, holder, false, GForm("%s::%s", vname, sname));
      }
    } 
  }
}

/**************************************************************************/
ZGeoNode* ZGeoNode::LocateNodeByPath(lStr_t& node_names)
{
  if(node_names.empty()) return this;

  string& name = node_names.front();
  ZGeoNode* next = dynamic_cast<ZGeoNode*>(GetElementByName(name));
  if(next == 0) {
    // printf("Create new node in LocateNode %s \n", name.c_str());
    next = new ZGeoNode(name.c_str());
    mQueen->CheckIn(next);
    Add(next);
  }
  
  node_names.pop_front();
  return next->LocateNodeByPath(node_names);
}



/**************************************************************************/
void ZGeoNode::CreateFaceset(GeoUserData* userdata)
{ 
  if( userdata && (userdata->fFaceSet == 0) && (mVol != 0)) {
    TBuffer3D*  buff = mVol->GetShape()->MakeBuffer3D();
    Float_t colorRGB[3] = {1, 0, 0};
    TGLFaceSet* fs = new TGLFaceSet(*buff, colorRGB, mSaturnID, mVol);
    userdata->fFaceSet = fs;
    mVol->SetField(userdata);
  }
}

/***************************************************************************/
void ZGeoNode::ClearThisList() 
{
  ZGeoNode* next; 
  for(lpZGlass_i i=mGlasses.begin(); i!=mGlasses.end(); ++i) {
    next = dynamic_cast<ZGeoNode*> (*i);
    if ( next->mVol) next->mVol->SetField(0);
  }
  ClearList();
}
