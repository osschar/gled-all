// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZGeoNode
//
// A ZGeoNode serves for representation of root TGeoNode
// object in Gled framework. 
//

#include "ZGeoNode.h"
#include "ZGeoNode.c7"

#include <Glasses/ZQueen.h>
#include <Stones/ZComet.h>

#include <TFile.h>
#include <TBuffer3D.h>
#include <TGeoNode.h>
#include <TColor.h>

typedef list<ZGeoNode*>           lpZGeoNode_t;
typedef list<ZGeoNode*>::iterator lpZGeoNode_i;

ClassImp(ZGeoNode)

/**************************************************************************/

void ZGeoNode::_init()
{
  // !!!! Set all links to 0 !!!! 
  mTNode     = 0;
  mNNodes    = 0;
  mNodeAlpha = 1;
  mDefFile   = "ZGeoNodes.root";
  bRnrSelf = true;
}

void ZGeoNode::_assert_tnode(const string& _eh, bool _ggeo_fallbackp)
{
  if(mTNode == 0)
    if(_ggeo_fallbackp && gGeoManager &&  gGeoManager->GetTopNode() )
      mTNode = gGeoManager->GetTopNode();
    else
      throw(_eh + "top-node can not be resolved.");
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
  lpZGeoNode_t dts; 
  CopyByGlass<ZGeoNode*>(dts);
  
  for(lpZGeoNode_i i=dts.begin(); i!=dts.end(); ++i) {
    GLensReadHolder _rlck(*i);
    (*i)->SetRnrSelf(true);
  }
}

void ZGeoNode::RnrOffForDaughters()
{ 
  lpZGeoNode_t dts; 
  CopyByGlass<ZGeoNode*>(dts);
  
  for(lpZGeoNode_i i=dts.begin(); i!=dts.end(); ++i) {
    GLensReadHolder _rlck(*i);
    (*i)->SetRnrSelf(false);
  }
}

/**************************************************************************/

void ZGeoNode::RnrOnRec()
{
  lpZGeoNode_t dts; 
  CopyByGlass<ZGeoNode*>(dts);
  for(lpZGeoNode_i i=dts.begin(); i!=dts.end(); ++i) {
    (*i)->RnrOnRec();
  }
  GLensReadHolder _rlck(this);
  SetRnrSelf(true);   
}

void ZGeoNode::RnrOffRec()
{ 
  lpZGeoNode_t dts; 
  CopyByGlass<ZGeoNode*>(dts);
  for(lpZGeoNode_i i=dts.begin(); i!=dts.end(); ++i) {
    (*i)->RnrOffRec();
  }
  GLensReadHolder _rlck(this);
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

void ZGeoNode::setup_color(Float_t alpha)
{
  Int_t ci = ((mTNode->GetColour() % 8) - 1) * 4;
  if(ci < 0) ci = 0;
  TColor* c = gROOT->GetColor(ci);
  if (c) {
    SetColor(c->GetRed(), c->GetGreen(), c->GetBlue(), alpha);
  } else {
    SetColor(1, 0, 0, alpha);
  }
}

/**************************************************************************/

ZGeoNode* ZGeoNode::insert_node(TGeoNode* geon, ZNode* holder, 
			       Bool_t rnr, const Text_t* title)
{
  // Creates ZGeoNode object from exported TGeoNode and adds it 
  // to mGlasses list.

  TGeoVolume* v = geon->GetVolume();
  ZGeoNode *nn = new ZGeoNode(geon->GetName(), title);
    
  nn->mTNode = geon;
  setup_ztrans(nn, geon->GetMatrix());
  nn->mTNodeName = geon->GetName();
  string m = v->GetMaterial()->GetName();
  int j = m.find_first_of("$");
  m = m.substr(0,j);
  nn->SetMat(m.c_str());
  nn->mNNodes = v && v->GetNodes() ? v->GetNodes()->GetSize() : 0;
  mQueen->CheckIn(nn);
  holder->Add(nn);
  nn->bRnrSelf = rnr;
  nn->AssertUserData();
  nn->setup_color(mNodeAlpha);
  
  // printf("%-16s %-6s  [%p]\n", geon->GetName(), title,  v);
  return nn;
}

/**************************************************************************/

void ZGeoNode::AssignGGeoTopNode()
{
  static const string _eh("ZGeoNode::AssignGGeoTopNode ");
  _assert_tnode(_eh, true);
}

/**************************************************************************/
// Importers
/**************************************************************************/

void ZGeoNode::ImportNodes()
{
  // Reads mTGeoNode and creates  
  // representative ZGeoNode node. 

  static const string _eh("ZGeoNode::ImportNodes ");

  _assert_tnode(_eh);

  RemoveLensesViaQueen(true);

  TIter next_node(mTNode->GetNodes());
  TGeoNode* geon;
  while((geon = (TGeoNode*)next_node())) {
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

    insert_node(geon, this, true,  GForm("%s::%s", vname, sname));
  }
}

/**************************************************************************/

void ZGeoNode::ImportByRegExp(const Text_t* target, TRegexp filter)
{
  // Imports mTGeoNode and groups the created nodes by given regular expression.

  static const string _eh("ZGeoNode::ImportByRegExp ");

  _assert_tnode(_eh);

  // split target into list of node names
  lStr_t node_names;
  GledNS::split_string(target, node_names, '/');
  ZGeoNode* holder = set_holder(node_names);

  TIter next_node(mTNode->GetNodes());
  TGeoNode* geon;

  while((geon = (TGeoNode*)next_node())) {
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

    insert_node(geon, holder, false,  GForm("%s::%s", vname.Data(), sname.Data()));
  }
}

/**************************************************************************/

void ZGeoNode::ImportNodesWCollect() 
{
  // Import mTGeoNode and groups the nodes by 
  // mother volume name.

  static const string _eh("ZGeoNode::ImportNodesWCollect ");

  _assert_tnode(_eh);
  
  RemoveLensesViaQueen(true);

  map<string, ZNode*> nmap;
  TIter next_node(mTNode->GetNodes());
  TGeoNode* geon;
  while((geon = (TGeoNode*)next_node())) {
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
    
    insert_node(geon, holder, false,  GForm("%s::%s", vname, sname));
  }
}

/**************************************************************************/

void ZGeoNode::ImportUnimported(const Text_t* target)
{  
  static const string _eh("ZGeoNode::ImportUnimported ");

  _assert_tnode(_eh);
   
  ZGeoNode* holder = dynamic_cast<ZGeoNode*>(GetElementByName(target));
  if ( holder == 0 ){
    // printf("create holder node %s \n", target);
    holder = new  ZGeoNode(target);
    mQueen->CheckIn(holder);
    Add(holder);
  }
 
  TIter next_node(mTNode->GetNodes());
  TGeoNode* geon;

  while((geon = (TGeoNode*)next_node())) {
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
        insert_node(geon, holder, false, GForm("%s::%s", vname, sname));
      }
      
      if (ud && ((ud->bIsImported) == false)) {
        insert_node(geon, holder, false, GForm("%s::%s", vname, sname));
      }
    } 
  }
}

/**************************************************************************/

void ZGeoNode::AssertUserData()
{ 
  // Creates TGLFaceSet object rendered by ZGeoNode_GL_Rnr
  // and saves it in TGeoVolume.  

  TGeoVolume* v = GetVolume();
  if( v ) {
    GeoUserData* userdata = dynamic_cast<GeoUserData*>(v->GetField());

    if (v->GetField() == 0) {
      userdata = new GeoUserData();
      v->SetField(userdata);
    }

    if (userdata->fFaceSet == 0) {    
      TGeoVolume* vol = GetVolume();
      TBuffer3D*  buff = GetVolume()->GetShape()->MakeBuffer3D();
      Float_t colorRGB[3] = {1, 0, 0};
      TGLFaceSet* fs = new TGLFaceSet(*buff, colorRGB, mSaturnID, vol);
      userdata->fFaceSet = fs;
      userdata->bIsImported = true;
      vol->SetField(userdata);
    }
  }
}

/***************************************************************************/

void ZGeoNode::SaveToFile(const Text_t* file)
{
  static const string _eh("ZGeoNode::SaveToFile ");
  
  if(file == 0) file = mDefFile.Data();
  ISdebug(1, _eh + "loading from '" + file + "'.");

  ZComet c("ZGeoNodes");
  lpZGlass_i i, end;
  BeginIteration(i, end);
  while(i != end) {
    c.AddTopLevel(*i, false, true, -1);
    ++i;
  }
  EndIteration();

  TFile f(file, "RECREATE");
  c.Write();
  f.Close();
}

void ZGeoNode::LoadFromFile(const Text_t* file)
{
  static const string _eh("ZGeoNode::LoadFromFile ");

  if(file == 0) file = mDefFile.Data();
  ISdebug(1, _eh + "loading from '" + file + "'.");

  RemoveLensesViaQueen(true);

  TFile f(file, "READ");
  auto_ptr<ZComet> c( dynamic_cast<ZComet*>(f.Get("ZGeoNodes")) );
  f.Close();
  if(c.get() == 0) {
    // This check should be more elaborate ... but need common infrastructure
    // anyway.
    throw(_eh + "could not read 'ZGeoNodes' from file '" + file + "'.");
  }
  mQueen->AdoptComet(this, 0, c.get());
}

void ZGeoNode::Restore()
{
  static const string _eh("ZGeoNode::Restore");
  ISdebug(1, GForm("%s", _eh.c_str()));
  // printf("restoring node %s:%p \n", GetName(), this);
  if (! mTNodeName.IsNull()) {
    // search TGeoVolume to set it to zgeonode
    TGeoNode* sp =  get_tnode_search_point();
    if(sp && locate_tnode(this, sp)){
      // !!!! check if userdata created
      // printf("createing Faceset reading TGeoNode %s \n", GetTNode()->GetName());
      AssertUserData();
    }
  }
  lpZGlass_i i, end;
  BeginIteration(i, end);
  ZGeoNode* nn;
  while(i != end) {
    if((nn = dynamic_cast<ZGeoNode*>(*i))) {
      nn->Restore();
    }
    ++i;
  }
  EndIteration();
}

/**************************************************************************/

TGeoNode* ZGeoNode::get_tnode_search_point()
{
  static const string _eh("ZGeoNode::GetTNodeSearchPoint");

  ZGeoNode* p = dynamic_cast<ZGeoNode*>(GetParent());
  if(p && p->GetTNode() != 0) {
    return p->GetTNode();
  } else {
    // printf("GetTNodeSearchPoint searching from TOP node \n");
    if(!gGeoManager) {
      ISerr(GForm("%s gGeoManager not set.", _eh.c_str()) );
      return 0;
    }
    return gGeoManager->GetTopNode();
  }
}

/**************************************************************************/

ZGeoNode* ZGeoNode::set_holder(lStr_t& node_names)
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
  return next->set_holder(node_names);
}

/************************************************************************/

Bool_t ZGeoNode::locate_tnode( ZGeoNode* zn, TGeoNode* cur_node)
{  
  // Searches TGeoNode from cur_node whith name zn->mTNodeName
  // and sets the mTNode value to zn. The function is needed when
  // restoring geometry from *.root files.
 
  TGeoVolume* vol = cur_node->GetVolume();

  TGeoNode* n = 0;
  if(vol->GetNodes()) {
    n = vol->FindNode(zn->GetTNodeName());
    if(n) {
      zn->SetTNode(n);
      return true;
    }
  }

  {
    TIter next_node(cur_node->GetVolume()->GetNodes());
    while((n = (TGeoNode*)next_node())) {
      if (locate_tnode(zn, n)){
	return true;
      }
    }
  }

  return false;
}

