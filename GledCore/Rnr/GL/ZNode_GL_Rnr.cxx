// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZNode_GL_Rnr.h"
#include <Glasses/ZRlNameRnrCtrl.h>
#include <RnrBase/RnrDriver.h>
#include <GledView/GLTextNS.h>
#include <Ephra/Saturn.h>

#include <FL/gl.h>

namespace GVNS = GledViewNS;
namespace OS   = OptoStructs;

#define PARENT ZGlass_GL_Rnr

/**************************************************************************/

void ZNode_GL_Rnr::_init()
{
  _setup_rnrmod();
}

void ZNode_GL_Rnr::_setup_rnrmod()
{
  if(fImg) {
    OS::lZLinkDatum_i ld = fImg->fLinkData.begin();
    while(ld != fImg->fLinkData.end()) {
      if(&ld->GetLinkRef() == &mNode->mRnrMod) {
	mRnrModLD = &(*ld);
	return;
      }
      ++ld;
    }
  }
  mRnrModLD = 0;
}

/**************************************************************************/

void ZNode_GL_Rnr::SetImg(OS::ZGlassImg* newimg)
{
  PARENT::SetImg(newimg);
  _setup_rnrmod();
}

/**************************************************************************/

void ZNode_GL_Rnr::CreateRnrScheme(RnrDriver* rd)
{
  ZNode& N = *mNode;
  GVNS::RnrBits& rb = fImg->fClassInfo->fViewPart->fDefRnrCtrl.fRnrBits;

  if(mRnrModLD && mRnrModLD->GetToGlass()) {
    int pre = 0, post = 0;
    if(N.bModSelf) {
      pre = rb.fSelf[1] ? rb.fSelf[1] : 4; // This should almost always be 4.
      post = pre + 1;
    }
    if(N.bModElements) {
      if(pre) {
	pre  = TMath::Min(pre,  int(rb.fList[3]));
	post = TMath::Max(post, int(rb.fList[3] + 1));
      } else {
	pre = rb.fList[3];
	post = pre + 1;
      }
    }
    // printf("Calculated %d %d for '%s'.\n", pre, post, mRnrModLD->GetToGlass()->GetName());
    if(pre) {
      A_Rnr* rm_rnr = rd->GetRnr(mRnrModLD->GetToImg());
      (*mRnrScheme)[pre].push_back(RnrElement(rm_rnr, &A_Rnr::PreDraw));
      (*mRnrScheme)[post].push_back(RnrElement(rm_rnr, &A_Rnr::PostDraw));
    }
  }

  crs_links(rd, mRnrScheme);
  if(N.bRnrSelf) {
    crs_self(rd, mRnrScheme);
  } else {
    (*mRnrScheme)[rb.fSelf[0]].push_back(RnrElement(this, &A_Rnr::PreDraw));
    (*mRnrScheme)[rb.fSelf[2]].push_back(RnrElement(this, &A_Rnr::PostDraw));
  }
  if(N.bRnrElements) crs_elements(rd, mRnrScheme);
}

/**************************************************************************/
// Basic GL Drawing
/**************************************************************************/
// Be reasonable when redefining Pre/Post Draw in derived classes

void ZNode_GL_Rnr::build_GL_mat()
{
  int s=0;
  for(Int_t i=1; i<=3; i++) {
    for(Int_t j=1; j<=4; j++)
      mGL_Mat[s++] = mNode->mTrans(j,i);
  }
  mGL_Mat[s++] = mNode->mTrans(1,4); mGL_Mat[s++] = mNode->mTrans(2,4);
  mGL_Mat[s++] = mNode->mTrans(3,4); mGL_Mat[s++] = mNode->mTrans(4,4);
}

/**************************************************************************/

void ZNode_GL_Rnr::PreDraw(RnrDriver* rd)
{
  PARENT::PreDraw(rd);
  if(mNode->mStampReqTrans > mStampTrans) {
    build_GL_mat();
    mStampTrans = mNode->mTimeStamp;
  }
  glPushAttrib(GL_TRANSFORM_BIT);
  glPushMatrix();
  glMultMatrixf(mGL_Mat);
  { // Scaling stuff
    bool norm_p = false;
    if(mNode->bUseScale) {
      glScalef(mNode->mSx, mNode->mSy, mNode->mSz);
      norm_p = true;
    }
    ZNode* pn = mNode->mParent;
    if(mNode->bUseOM && pn && (ZGlass*)pn==rd->TopPM() && pn->bUseOM) {
      Float_t dom =  mNode->mOM - pn->mOM;
      if(dom != 0) {
	Double_t s = TMath::Power(10, dom);
	glScaled(s, s, s);
	norm_p = true;
      }
    }
    if(norm_p) glEnable(GL_NORMALIZE);
  }
  rd->PushPM(mNode);
}

void ZNode_GL_Rnr::PostDraw(RnrDriver* rd)
{
  if(rd->GetRnrNames() == true && bSuppressNameLabel == false && mNode->mName != "") {
    string name( mNode->GetName() );
    GLTextNS::RnrTextBar(rd, name);
  }

  rd->PopPM();
  glPopMatrix();
  glPopAttrib();
  PARENT::PostDraw(rd);
}

