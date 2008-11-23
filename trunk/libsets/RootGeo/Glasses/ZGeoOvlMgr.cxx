// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
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

#include <TMath.h>
#include <TGeoOverlap.h>
#include <TPolyMarker3D.h>

ClassImp(ZGeoOvlMgr);

/**************************************************************************/

void ZGeoOvlMgr::_init()
{
  mMotherCol.rgba(  0,   1, 0.8, 0.6);
  mExtrCol.  rgba(0.3,   0,   1,   1);
  mOvlpCol1. rgba(  1,   1,   1,   1);
  mOvlpCol2. rgba(  1,   1,   0,   1);
  mPMExtrCol.rgba(  1,   0,   0,   1);
  mPMOvlpCol.rgba(  1,   0,   1,   1);
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

  // throw(_eh + "code not in sync with changes in ROOT, aborting.");
  printf("Import START eps=%f \n", mResol);

  TIter next_node(lOverlaps);
  TGeoOverlap *ovl;
  TGeoIterator top_git   (gGeoManager->GetTopVolume());
  TGeoNode*    top_gnode (top_git.Next());

  // go through the list of overlaps  locate each node in
  // absolute coordinates
  while((ovl = (TGeoOverlap*)next_node())) {
    Bool_t isExtr = ovl->IsExtrusion();
    Bool_t isOvlp = ovl->IsOverlap();
    ISdebug(1, GForm("%sScanning for Extp=%d, Ovlp=%d: vol1=%-12s vol2=%-12s",
                     _eh.Data(), isExtr, isOvlp,
                     ovl->GetFirstVolume()->GetName(),
                     ovl->GetSecondVolume()->GetName()));

  reiterate:
    TGeoNode    *n1 =  0, *n2 =  0, *gnode;
    TGeoVolume  *v1 =  0, *v2 =  0, *gvol;
    Int_t        l1 = -1,  l2 = -1;
    TGeoIterator git(top_git);
    gnode = top_gnode;
    do {
      gvol = gnode->GetVolume();
      if(gvol == ovl->GetFirstVolume()) {
        top_git = git; top_gnode = gnode;
        n1 = gnode; v1 = gvol; l1 = git.GetLevel();
        ISdebug(2, GForm("  Found first  vol lvl=%d", l1));
        if(isOvlp)
          git.Skip();

        while((gnode = git.Next()) != 0) {

          gvol = gnode->GetVolume();
          if(gvol == ovl->GetSecondVolume()) {
            n2 = gnode; v2 = gvol; l2 = git.GetLevel();
            ISdebug(2, GForm("  Found second vol lvl=%d", l2));

            Int_t       motherl;
            TGeoNode*   mothern;
            TGeoVolume* motherv;
            if(isExtr) {
              motherl = l1;
              mothern = n1;
              motherv = v1;
            } else {
              motherl = TMath::Min(l1, l2);
              do {
                --motherl;
                mothern = motherl > 0 ? git.GetNode(motherl) : top_node;
                motherv = mothern->GetVolume();
              } while(motherv->IsAssembly());

            }
            TGeoHMatrix motherm;
            {
              TGeoNode *node = git.GetTopVolume()->GetNode(git.GetIndex(1));
              motherm.Multiply(node->GetMatrix());
              for (Int_t i=2; i<=motherl; i++) {
                node = node->GetDaughter(git.GetIndex(i));
                motherm.Multiply(node->GetMatrix());
              }
            }

            TString mname  = isExtr ? "Extr: " : "Ovlp: ";
            mname += motherv->GetName();
            TString mtitle = top_node->GetVolume()->GetName();
            for(Int_t l=1; l<motherl; ++l) {
              mtitle += "/";
              mtitle += git.GetNode(l)->GetVolume()->GetName();
            }

            ISdebug(1, GForm("%sImporting %s '%s' from %s", _eh.Data(),
                             mname.Data(), mtitle.Data(),
                             ovl->GetName()));

            ZGeoOvl* zm = new ZGeoOvl(mname, mtitle);
            zm->SetIsExtr ( isExtr );
            zm->SetOverlap( ovl->GetOverlap() );

            TPolyMarker3D* pm = ovl->GetPolyMarker();
            zm->SetPM_N  ( pm->GetLastPoint() );
            zm->SetPM_p  ( pm->GetP() );
            zm->SetPM_ColByRef( isExtr ? mPMExtrCol : mPMOvlpCol );
            zm->SetColorByRef ( mMotherCol );
            {
              GLensWriteHolder wlck(this);
              mQueen->CheckIn(zm);
              Add(zm);
            }
            zm->setup_ztrans(zm, &motherm);
            zm->SetRnrNode(isExtr);
            zm->SetTNode(mothern);
            zm->AssertUserData();

            if(isExtr) {
              ZGeoNode* zn = create_standalone_node(v2->GetName(), 0, n2, ovl->GetSecondMatrix());
              zn->SetColorByRef( mExtrCol );

              GLensWriteHolder wlck(zm);
              zm->Add(zn);
            } else {
              ZGeoNode* zn1 = create_standalone_node(v1->GetName(), 0, n1, ovl->GetFirstMatrix());
              zn1->SetColorByRef( mOvlpCol1 );
              ZGeoNode* zn2 = create_standalone_node(v2->GetName(), 0, n2, ovl->GetSecondMatrix());
              zn2->SetColorByRef( mOvlpCol2 );

              GLensWriteHolder wlck(zm);
              zm->Add(zn1);
              zm->Add(zn2);
            }

            break;
          }
        }
        break;
      }
    } while((gnode = git.Next()) != 0);

    if(v2 == 0) {
      ISdebug(2, "  Could not find both volumes, resetting geo-iterator.");
      top_git.Reset();
      top_gnode = top_git.Next();
      goto reiterate;
    }

  }
  printf("Import END eps=%f \n", mResol);
}

/**************************************************************************/

void ZGeoOvlMgr::RecalculateOvl()
{
  RemoveLensesViaQueen(true);

  if (gGeoManager) {
    gGeoManager->GetTopNode()->CheckOverlaps(mResol);
    if (mMinOvl > mResol) mMinOvl=mResol;
    ImportOverlaps(gGeoManager->GetListOfOverlaps(), gGeoManager->GetTopNode());
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
//
// These are not really needed with the new overlap import code (9.6.2006).
// Left here as their functionality might be needed in the future.
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

ZGeoNode* ZGeoOvlMgr::create_standalone_node(const Text_t* n, const Text_t* t,
					    TGeoNode* gnode, TGeoMatrix* gmatrix)
{
  ZGeoNode *nn = new ZGeoNode(n, t);

  if (gnode) {
    TGeoVolume* v = gnode->GetVolume();
    nn->SetTNode(gnode);
    if (gmatrix) nn->setup_ztrans(nn, gmatrix);
    TString m = v->GetMaterial()->GetName();
    int j = m.First('$');
    if (j != kNPOS)
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

