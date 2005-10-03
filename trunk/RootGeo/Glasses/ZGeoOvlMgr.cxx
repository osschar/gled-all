// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZGeoOvlMgr
//
// A graphical representation of overlaping nodes which
// are exported from gGeoManager fOverlaps list.

#include "ZGeoOvlMgr.h"
#include "ZGeoOvlMgr.c7"
#include <Glasses/ZQueen.h>

#include <TGeoOverlap.h>
#include <TPolyMarker3D.h>

ClassImp(ZGeoOvlMgr)

/**************************************************************************/

void ZGeoOvlMgr::_init()
{
  // !!!! Set all links to 0 !!!!
  mMotherCol.rgba(0, 1, 0.8, 0.3);
  mOvlCol1.rgba(1, 1, 1, 0.8);
  mOvlCol2.rgba(1, 1, 0, 0.8);
  mExtrCol.rgba(0.3, 0, 1, 0.8);
  mPM_Col.rgba(1, 0, 0, 1);
  mResol = 0.1;
  mMaxOvl = 100;
  mMinOvl = mResol;
}

/*************************************************************************/

void ZGeoOvlMgr::RnrOvlInterval()
{
  // Set mRnrSelf to true in nodes which have mOverlap
  // value between  mMinOvl amd mMaxOvl.

  list<ZGeoNode*> gns; CopyListByGlass<ZGeoNode>(gns);
  for(list<ZGeoNode*>::iterator i=gns.begin(); i!=gns.end(); ++i) {
    list<ZGeoOvl*> ol; (*i)->CopyListByGlass<ZGeoOvl>(ol);
    Bool_t rnr_top_p = false;
    for(list<ZGeoOvl*>::iterator k=ol.begin(); k!=ol.end(); ++k) {
      ZGeoOvl* n = *k;
      GLensReadHolder _rlck(n);
      Bool_t rnr_p = n->GetOverlap() < mMaxOvl && n->GetOverlap() > mMinOvl;
      n->SetRnrSelf(rnr_p);  n->SetRnrElements(rnr_p);
      rnr_top_p = rnr_top_p || rnr_p;
    }
    GLensReadHolder _rlck(*i);
    (*i)->SetRnrSelf(rnr_top_p);
    (*i)->SetRnrElements(rnr_top_p);
  }
}

/**************************************************************************/

void ZGeoOvlMgr::ImportOverlaps(TObjArray* lOverlaps, TGeoNode* top_node)
{
  // Creates a list of TGeoOvl objects reading 
  // gGeoManager list fOverlaps.

  static const Exc_t _eh("ZGeoOvlMgr::ImportOverlaps");
  ISdebug(1, GForm("%s, resolution %f", _eh.Data(), mResol));

  //  printf("Import START %f \n", mResol);
  TIter next_node(lOverlaps);
  TGeoOverlap     *ovl;
  TGeoNode        *n1 = 0;
  ZGeoNode        *ovlm;
  ZGeoOvl         *ovln = 0;
  TPolyMarker3D*   pm;
  const Text_t    *mn;
  TString           mt;
  map<TString, ZGeoNode*> nmap;

  // go through the list of overlaps  locate each node in 
  // absolute coordinates
  while((ovl = (TGeoOverlap*)next_node())) {
    TGeoVolume* motherv = ovl->GetVolume();
    const char* mname = motherv->GetName(); 
    // printf ("Importing %s %s Extr(%d).\n", mname, ovl->GetName(), ovl->IsExtrusion());
    ISdebug(1, GForm("%s Importing %s %s Extr(%d)", _eh.Data(), mname, ovl->GetName(), ovl->IsExtrusion()));
    
    map<TString, ZGeoNode*>::iterator i = nmap.find(motherv->GetName());
    if(i == nmap.end()) {     
      ovlm = new ZGeoNode(mname);
      // printf("creating mother node %s \n", ovlm->GetName());
      {
	GLensWriteHolder wlck(this);
	mQueen->CheckIn(ovlm);
	Add(ovlm);
      }

      mt = setup_absolute_matrix(top_node,motherv, ovlm);
      ovlm->setup_color(mNodeAlpha);
      ovlm->SetTitle(mt.Data()); 
      ovlm->AssertUserData();
      nmap[mname] = ovlm;
    } 
    else {
      // printf("mother node allrady eixist for volume %s \n", motherv->GetName());
      ovlm = i->second;
    }

    if (ovl->IsExtrusion()){
      n1 = ovl->GetNode(0);
      // printf("is extrusion %s \n ", n1->GetVolume()->GetName());
      // printf("number of nodes ... %d \n", ovlm->Size());
      mn = GForm("%s::extr%d",n1->GetVolume()->GetName(), ovlm->Size());
      ovln = create_standalone_node(mn, n1->GetName(), n1);
      // TString tname = mt + '/' + n1->GetName();
      // ovln->SetTitle(tname.Data());      
      // ovln->SetTrans(get_ztrans(n1->GetMatrix()));
      setup_ztrans(ovln, n1->GetMatrix());
      ovln->mIsExtr = true;
    } else {
      mn = GForm("overlap%d",ovlm->Size());
      ovln = create_standalone_node(mn, "holder");
      // create the overlaping nodes
      insert_node(ovl->GetNode(0), ovln, GForm("%s",ovl->GetNode(0)->GetName()));
      // TString tname = mt + '/' + ovl->GetNode(0)->GetName();
      // ovln->First()->SetTitle(tname.Data());
      
      insert_node(ovl->GetNode(1), ovln, GForm("%s",ovl->GetNode(1)->GetName() ));
      // tname = mt + '/' + ovl->GetNode(1)->GetName();
      // ovln->Last()->SetTitle(tname.Data());
    }

    pm = ovl->GetPolyMarker();
    ovln->SetPM_N(pm->GetLastPoint());
    ovln->SetPM_p(pm->GetP());
    ovln->mPM_Col = mPM_Col;
    ovln->mOverlap = ovl->GetOverlap();
    ovlm->SetColor(mMotherCol.r(), mMotherCol.g(),mMotherCol.b(),mMotherCol.a());
    setup_zcolor(ovln);
    ovlm->Add(ovln);
  }  
  // printf("Import END %f \n", mResol);

}

/**************************************************************************/

void ZGeoOvlMgr::RecalculateOvl()
{
  RemoveLensesViaQueen(true);

  if (gGeoManager) {
    gGeoManager->CheckOverlaps(mResol);
    if (mMinOvl > mResol) mMinOvl=mResol;    
    ImportOverlaps(gGeoManager->GetListOfOverlaps() , gGeoManager->GetTopNode());
  }
}

/**************************************************************************/

void ZGeoOvlMgr::Restore()
{
  // Sets UserData to nodes with mTNode set.
  // The list was created by calling function LoadFromFile. 

  static const Exc_t _eh("ZGeoOvlMgr::Restore");
  ISdebug(1, GForm("%s START", _eh.Data()));

  Stepper<ZGeoNode> mn(this);
  while(mn.step()) {
    TString path = mn->GetTitle();
    path = path(1, path.Length() - 1);

    // printf("search TNode for mother path: %s \n", path.Data());
    Ssiz_t end = path.Length();
    Ssiz_t pos = path.First("/");
     
    TGeoNode* tn = gGeoManager->GetTopNode();

    // check if mother tnod is not top node , that is /ALIC
    if(pos != kNPOS) {
      path = path(pos, end - pos);
      set_tnode_by_path(path, tn);
      // printf("Tnode in Restore %s \n", tn->GetName());
    }

    if(tn) {
      mn->SetTNode(tn);
      mn->AssertUserData();

      Stepper<ZGeoOvl> ovl(*mn);
      while(ovl.step()) {
	ovl->Restore( mn->GetVolume());
      }
    }
  }
  ISdebug(1, GForm("%s END", _eh.Data()));
}

/**************************************************************************/

void ZGeoOvlMgr::DumpOvlMgr()
{
  list<ZGeoNode*> gns;
  CopyListByGlass<ZGeoNode>(gns);
  for(list<ZGeoNode*>::iterator i=gns.begin(); i!=gns.end(); ++i) {
    list<ZGeoOvl*> ol;
    (*i)->CopyListByGlass<ZGeoOvl>(ol);
    for(list<ZGeoOvl*>::iterator k=ol.begin(); k!=ol.end(); ++k) {
      (*k)->DumpOvl();
    }
  }
}

/**************************************************************************/
// Protected methods
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

/**************************************************************************/

Bool_t ZGeoOvlMgr::locate_first_from_top(TGeoNode* cur_node, TGeoVolume* vol,
					 ZGeoNode* zn, lgn_t& result)
{
  if (cur_node->GetVolume() == vol) {
    zn->SetTNode(cur_node);
    zn->SetTNodeName(cur_node->GetName());
    result.push_back(cur_node);
    return true;
  }

  TGeoNode* n;
  TIter next_node(cur_node->GetVolume()->GetNodes());
  while ((n = (TGeoNode*)next_node())) {
    bool found = locate_first_from_top(n, vol, zn, result);
    if (found) {
      result.push_back(cur_node);
      return true;
    }
  }
  return false;
}

/*************************************************************************/

TString ZGeoOvlMgr::setup_absolute_matrix(TGeoNode* top_node, TGeoVolume* vol,
					 ZGeoNode* zn)
{
  lgn_t  path;
  ZTrans  mt;
  TGeoNode* gn;
  //  printf(" ZGeoOvlMgr::setup_absolute_matrix %s \n", zn->GetName());
  locate_first_from_top(top_node, vol, zn, path);
  // printf("setup_absolute_matrix zn->GetTNode()->GetName() \n", zn->GetTNode()->GetName());
  TString title;

  for(lgn_ri k=path.rbegin(); k!=path.rend(); ++k) { 
    gn = *k;
    mt *= get_ztrans(gn->GetMatrix());
    title += '/';
    title += gn->GetName();
  }
  zn->SetTrans(mt);
  return title;
}

/**************************************************************************/

void ZGeoOvlMgr::setup_zcolor(ZGeoOvl* ovlm)
{
  // Sets color to ZGeoOvl node. If node is extrusion, the color is
  // set to mExtrCol. In case node represents overlap, the color of the 
  // first overlaping node in ovlm is  mOvlCol1 and the color of the second
  // is mOvlCol2.

  if (ovlm->mIsExtr) {
    ovlm->SetColor(mExtrCol.r(), mExtrCol.g(), mExtrCol.b(), mExtrCol.a());
  } else {
    ZGeoNode* n = (ZGeoNode*)ovlm->FrontElement();
    n->mColor = mOvlCol1;

    n = (ZGeoNode*)ovlm->BackElement();
    n->mColor = mOvlCol2;
  }
}

/**************************************************************************/

ZGeoOvl* ZGeoOvlMgr::create_standalone_node(const Text_t* n, const Text_t* t,
					    TGeoNode* tn)
{
  ZGeoOvl *nn = new ZGeoOvl(n, t);
 
  if (tn) {
    TGeoVolume* v = tn->GetVolume();
    nn->SetTNode(tn);
    TString m = v->GetMaterial()->GetName();
    int j = m.First('$');
    if(j != kNPOS)
      m = m(0,j);
    nn->SetMaterial(m.Data());
    nn->AssertUserData();
  }

  mQueen->CheckIn(nn);
  return nn;
}


void ZGeoOvlMgr::set_tnode_by_path(TString path, TGeoNode*& gn)
{
  static const Exc_t _eh("ZGeoOvlMgr::SetTNodeByPath");

  Bool_t last_token;
  if (path.First('/') == path.Last('/')) {
    last_token=true;
  } else {
    last_token=false;
  }

  path = path(1, path.Length() - 1);  //remove first slash in path
  Ssiz_t pos  = path.First('/');

  if (last_token) {
    TGeoVolume* vol = gn->GetVolume();
    if (vol->GetNodes()) {
      // printf("search  %s in volume %s \n", path.Data(), vol->GetName());
      TGeoNode* n = vol->FindNode(path.Data());
      if (n) {
	gn = n;
	// printf("TGeoNode %s found from path  %s in volume %s \n", gn->GetName(), path.Data(), vol->GetName());
      }
      else {
	ISerr(GForm("%s Can't find reference for %s in gGeoManager.", _eh.Data(),path.Data()));
      }
    }
  } else {
    if (pos != kNPOS) {
      TString nname = path(0, pos);
      path = path(pos, path.Length() - pos);   
      // printf("temp %s \n", path.Data());
      TGeoVolume* vol = gn->GetVolume();
      if (vol->GetNodes()) {
	TGeoNode* n = vol->FindNode(nname.Data());
	if (n) {
	  gn = n;
	  set_tnode_by_path(path, gn);
	}
      }
    }
  }
}

