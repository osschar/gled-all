// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZNode_GL_Rnr.h"
#include <Glasses/ZRlNodeMarkup.h>
#include <Rnr/GL/GLRnrDriver.h>
#include <GledView/GLTextNS.h>
#include <Ephra/Saturn.h>

#include <GL/gl.h>

namespace GVNS = GledViewNS;
namespace OS   = OptoStructs;

#define PARENT ZGlass_GL_Rnr

/**************************************************************************/

void ZNode_GL_Rnr::_init()
{
  _setup_rnrmod();
  mPMSE.fNode = mNode;
  mExDOM = 0;
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
  ZTrans& T(mPMSE.fLocal);
  int s=0;
  for(Int_t i=1; i<=3; i++) {
    for(Int_t j=1; j<=4; j++)
      mGL_Mat[s++] = T(j,i);
  }
  mGL_Mat[s++] = T(1,4); mGL_Mat[s++] = T(2,4);
  mGL_Mat[s++] = T(3,4); mGL_Mat[s++] = T(4,4);
}

/**************************************************************************/

void ZNode_GL_Rnr::PreDraw(RnrDriver* rd)
{
  PARENT::PreDraw(rd);

  PMSEntry& prev_pmse = rd->TopPM();
  Float_t dom = 0;
  if(mNode->bUseOM && prev_pmse.fNode && prev_pmse.fNode->bUseOM) {
    dom = mNode->mOM - prev_pmse.fNode->mOM;
  }

  if(mNode->mStampReqTrans > mStampTrans || dom != mExDOM) {
    mPMSE.fLocal = mNode->mTrans;
    bNormP = false;
    if(mNode->bUseScale) {
      mPMSE.fLocal.Scale3(mNode->mSx, mNode->mSy, mNode->mSz);
      bNormP = true;
    }
    if(dom != 0) {
      const Double_t s = TMath::Power(10, dom);
      mPMSE.fLocal.Scale3(s, s, s);
      bNormP = true;
    }
    mExDOM = dom;
    build_GL_mat();
    mStampTrans  = mNode->mTimeStamp;
  }
  glPushMatrix();
  glMultMatrixd(mGL_Mat);
  if(bNormP) {
    Bool_t bNormWasOffP = !glIsEnabled(GL_NORMALIZE);
    if(bNormWasOffP) glEnable(GL_NORMALIZE);
  }

  mPMSE.bTo = false; mPMSE.bFrom = false; // Could be optimized if rnr and pmse separated.
  rd->PushPM(mPMSE);
}

void ZNode_GL_Rnr::PostDraw(RnrDriver* rd)
{
  if(rd->GL()->GetMarkupNodes() == true) {
    RNRDRIVER_GET_RNRMOD_LENS(nrc, rd, ZRlNodeMarkup);
    if(nrc_lens->GetRnrNames() && bSuppressNameLabel == false &&
       mNode->mName != "")
      {
	string name( mNode->GetName() );
	GLTextNS::RnrTextBar(rd, name);
      }
    if(nrc_lens->GetRnrAxes()) {
      glBegin(GL_LINES);
      glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(1,0,0);
      glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,1,0);
      glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,1);
      glEnd();
    }
  }

  rd->PopPM();
  if(bNormP && bNormWasOffP) glDisable(GL_NORMALIZE);
  glPopMatrix();
  PARENT::PostDraw(rd);
}

