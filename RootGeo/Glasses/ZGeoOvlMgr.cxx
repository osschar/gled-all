// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZGeoOvlMgr
//
//

#include "ZGeoOvlMgr.h"
#include "ZGeoOvlMgr.c7"
#include <Glasses/ZQueen.h>

#include <TGeoOverlap.h>
#include <TPolyMarker3D.h>

typedef list<ZGeoNode*>           lpZGNode_t;
typedef list<ZGeoNode*>::iterator lpZGNode_i;

ClassImp(ZGeoOvlMgr)


/**************************************************************************/
void ZGeoOvlMgr::_init()
{
  // !!!! Set all links to 0 !!!!
  mMotherCol.rgba(0, 1, 0.8, 0.3);
  mOvlCol1.rgba(1, 1, 0, 0.8);
  mOvlCol2.rgba(1, 1, 1, 0.8);
  mExtrCol.rgba(0.3, 0, 1, 0.8);
  mPM_Col.rgba(1, 0, 0, 1);
  mResol = 0.1;
  mMaxOvl = 100;
  mMinOvl = mResol;
}

/**************************************************************************/
void ZGeoOvlMgr::Dump()
{
  list<ZGeoNode*> gns;
  CopyByGlass<ZGeoNode*>(gns);
  for(list<ZGeoNode*>::iterator i=gns.begin(); i!=gns.end(); ++i) {
    list<ZGeoOvl*> ol;
    (*i)->CopyByGlass<ZGeoOvl*>(ol);
    for(list<ZGeoOvl*>::iterator k=ol.begin(); k!=ol.end(); ++k) {
      (*k)->Dump();
    }
  }
}

/*************************************************************************/
ZTrans ZGeoOvlMgr::get_ztrans(TGeoMatrix* gm)
{
  const Double_t* rm = gm->GetRotationMatrix();
  const Double_t* tv = gm->GetTranslation();
  ZTrans t;
  t(1,1) = rm[0]; t(1,2) = rm[1]; t(1,3) = rm[2];
  t(2,1) = rm[3]; t(2,2) = rm[4]; t(2,3) = rm[5];
  t(3,1) = rm[6]; t(3,2) = rm[7]; t(3,3) = rm[8];
  t(1,4) = tv[0]; t(2,4) = tv[1]; t(3,4) = tv[2];
  return t;
}

/*************************************************************************/
Bool_t ZGeoOvlMgr::DeleteList(ZGeoNode* n){
  lpZGNode_t alist;
  n->CopyByGlass<ZGeoNode*>(alist);
  ZGeoNode* nn;

  for( lpZGNode_i i=alist.begin(); i!=alist.end(); ++i) {
    nn = *i;
    printf("erasing element %s from list in %s \n", nn->GetName(), n->GetName());
    DeleteList(nn);
  }

  {
    GLensWriteHolder wlck(this);
    n->ClearList();
    mQueen->RemoveLens(n);
  }
  return true;
}

/**************************************************************************/
Bool_t ZGeoOvlMgr::LocateFirstFromTop(TGeoNode* cur_node, TGeoVolume* search_vol, lgn_t& result)
{
  if(cur_node->GetVolume() == search_vol) {
    result.push_back(cur_node);
    return true;
  }

  TGeoNode* n;
  TIter next_node(cur_node->GetVolume()->GetNodes());
  while (n = (TGeoNode*)next_node()) {
    bool found = LocateFirstFromTop(n, search_vol, result);
    if( found ) {
      result.push_back(cur_node);
      return true;
    }
  }
  return false;
}

/*************************************************************************/
string ZGeoOvlMgr::setup_absolute_matrix(TGeoNode* top_node, ZGeoNode* zn)
{
  lgn_t  path;
  ZTrans  mt;
  TGeoNode* gn;
  LocateFirstFromTop(top_node, zn->GetVol(), path);
  string title;

  for(lgn_ri k=path.rbegin(); k!=path.rend(); ++k) { 
    gn = *k;
    mt *= get_ztrans(gn->GetMatrix());
    title += '/';
    title += gn->GetName();
  }
  zn->SetTrans(mt);
  return title;
}


void ZGeoOvlMgr::RnrOvlInterval()
{
  list<ZGeoNode*> gns;
  CopyByGlass<ZGeoNode*>(gns);
  for(list<ZGeoNode*>::iterator i=gns.begin(); i!=gns.end(); ++i) {
    list<ZGeoOvl*> ol;
    (*i)->CopyByGlass<ZGeoOvl*>(ol);
    for(list<ZGeoOvl*>::iterator k=ol.begin(); k!=ol.end(); ++k) {
      ZGeoOvl* n = *k;
      if ( (n->GetOverlap() < mMaxOvl) && (n->GetOverlap() > mMinOvl) ){
	n->SetRnrSelf(true); (*i)->SetRnrSelf(true);
      } else {
	n->SetRnrSelf(false);(*i)->SetRnrSelf(false);
      }
    }
  }
}

/**************************************************************************/
void ZGeoOvlMgr::ImportUnReplicated( TObjArray* lOverlaps, TGeoNode* top_node )
{
  TIter next_node(lOverlaps);
  TGeoOverlap     *ovl;
  TGeoNode        *n1 = 0;
  ZGeoNode        *ovlm;
  ZGeoOvl         *ovln = 0;
  TPolyMarker3D*   pm;
  const Text_t    *mn;
  string           mt;
  map<string, ZGeoNode*> nmap;

  // go through the list of overlaps  locate each node in 
  // absolute coordinates
  while(ovl = (TGeoOverlap*)next_node()) {
    TGeoVolume* motherv = ovl->GetVolume();
    const char* mname = motherv->GetName(); 
    // printf ("Importing %s %s Extr(%d).\n", mname, ovl->GetName(), ovl->IsExtrusion());

    map<string, ZGeoNode*>::iterator i = nmap.find(motherv->GetName());
    if(i == nmap.end()) {
      // create mother ZGeoNode 
      ovlm = new ZGeoNode(motherv->GetName());
      ovlm->SetVol(motherv);
      string m = motherv->GetMaterial()->GetName();
      int j = m.find_first_of("$");
      m = m.substr(0,j);
      ovlm->SetMat(m.c_str());
      ovlm->SetRnrSelf(false);

      {
	GLensWriteHolder wlck(this);
	mQueen->CheckIn(ovlm);
	Add(ovlm);
      }

      GeoUserData* userdata = new GeoUserData(true, 0);
      ovlm->CreateFaceset(userdata);
      ovlm->setup_color(mNodeAlpha);

      nmap[mname] = ovlm;
      mt = setup_absolute_matrix(top_node, ovlm);
      ovlm->SetTitle(mt.c_str());
    } 
    else {
      ovlm = i->second;
    }

    if (ovl->IsExtrusion()){
      n1 = ovl->GetNode(0);
      mn = GForm("%s::extr%d",n1->GetVolume()->GetName(), ovlm->Size());
      ovln = CreateStandaloneZNode(mn, n1->GetName(), n1->GetVolume());
      ovln->SetTrans(get_ztrans(n1->GetMatrix()));
      ovln->mIsExtr = true;
    } else {
      mn = GForm("overlap%d",ovlm->Size());
      ovln = CreateStandaloneZNode(mn, "holder");
      // create the overlaping nodes
      InsertNode(ovl->GetNode(0), ovln, false,  GForm("%s",ovl->GetNode(0)->GetName()));
      InsertNode(ovl->GetNode(1), ovln, false,  GForm("%s",ovl->GetNode(1)->GetName() ));
    }

    pm = ovl->GetPolyMarker();
    ovln->SetPM_N(pm->GetLastPoint());
    ovln->SetPM_p(pm->GetP());
    ovln->mPM_Col = mPM_Col;
    ovln->mRnrMark = false;
    ovln->mOverlap = ovl->GetOverlap();
    ovlm->SetColor(mMotherCol.r(), mMotherCol.g(),mMotherCol.b(),mMotherCol.a());
    setup_zcolor(ovln);
    ovlm->Add(ovln);
  }
}

/************************************************************/
void ZGeoOvlMgr::setup_zcolor(ZGeoOvl* ovlm){

  if(ovlm->mIsExtr){
    ovlm->SetColor(mExtrCol.r(), mExtrCol.g(), mExtrCol.b(), mExtrCol.a());
  }
  else { 
    ZGeoNode* n = (ZGeoNode*)ovlm->First();
     n->mColor = mOvlCol2;

    n = (ZGeoNode*)ovlm->Last();
    n->mColor = mOvlCol1;
  }
}

/************************************************************/
ZGeoOvl* ZGeoOvlMgr::CreateStandaloneZNode(const Text_t* n, const Text_t* t, TGeoVolume* v)
{
  ZGeoOvl *nn = new ZGeoOvl(n, t);
  if (v ){
    nn->SetVol(v);
    string m = v->GetMaterial()->GetName();
    int j = m.find_first_of("$");
    m = m.substr(0,j);
    nn->SetMat(m.c_str());
  }
  // rendering data
  GeoUserData* userdata = new GeoUserData(true, 0);
  nn->CreateFaceset(userdata);

  mQueen->CheckIn(nn);
  nn->bRnrSelf = false;
  return nn;
}

/**************************************************************/
void ZGeoOvlMgr::RecalculateOvl(){
  list<ZGeoNode*> gns;
  CopyByGlass<ZGeoNode*>(gns);
  {
    GLensWriteHolder wlck(this);
    ClearList();
  }
  for(list<ZGeoNode*>::iterator i=gns.begin(); i!=gns.end(); ++i) { 
    DeleteList(*i);
  }

  if (mGeoManager) {
    mGeoManager->CheckOverlaps(mResol);
    ImportUnReplicated(mGeoManager->GetListOfOverlaps() , mGeoManager->GetTopNode());
  }
}
