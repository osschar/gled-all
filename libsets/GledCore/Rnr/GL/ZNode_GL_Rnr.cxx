// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZNode_GL_Rnr.h"
#include <Glasses/ZRlNodeMarkup.h>
#include <Rnr/GL/GLRnrDriver.h>
#include <Rnr/GL/GLTextNS.h>
#include <Ephra/Saturn.h>

#include <TMath.h>

#include <GL/glew.h>

namespace GNS = GledNS;
namespace OS  = OptoStructs;

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
      if(&ld->GetLinkRef() == mNode->mRnrMod.ptr_link()) {
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
  GNS::RnrBits& rb = fImg->GetCI()->fDefRnrCtrl.fRnrBits;

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
      mPMSE.fLocal.Scale(mNode->mSx, mNode->mSy, mNode->mSz);
      bNormP = true;
    }
    if(dom != 0) {
      const Double_t s = TMath::Power(10, dom);
      mPMSE.fLocal.Scale(s, s, s);
      bNormP = true;
    }
    mExDOM = dom;
    mStampTrans  = mNode->mTimeStamp;
  }
  glPushMatrix();
  glMultMatrixd(mPMSE.fLocal.Array());
  if(bNormP) {
    bNormWasOffP = !glIsEnabled(GL_NORMALIZE);
    if(bNormWasOffP) glEnable(GL_NORMALIZE);
  }

  mPMSE.bTo = false; mPMSE.bFrom = false; // Could be optimized if rnr and pmse separated.
  rd->PushPM(mPMSE);
}

void ZNode_GL_Rnr::PostDraw(RnrDriver* rd)
{
  if(rd->GL()->GetMarkupNodes() == true) {
    GL_Capability_Switch light_off(GL_LIGHTING, 0);

    RNRDRIVER_GET_RNRMOD_LENS(nrc, rd, ZRlNodeMarkup);
    if(nrc_lens->GetRnrNames() && bSuppressNameLabel == false &&
       mNode->mName != "")
      {
	TString name( mNode->GetName() );
	GLTextNS::RnrTextBar(rd, name);
      }
    if(nrc_lens->GetRnrAxes()) {
      Float_t w = nrc_lens->GetAxeWidth(), wold,
	      l = nrc_lens->GetAxeLength();
      if(w != 0) {
	glGetFloatv(GL_LINE_WIDTH, &wold);
	glLineWidth(w);
      }
      glBegin(GL_LINES);
      glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(l,0,0);
      glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,l,0);
      glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,l);
      glEnd();
      if(w != 0) {
	glLineWidth(wold);
      }
    }
  }

  rd->PopPM();
  if(bNormP && bNormWasOffP) glDisable(GL_NORMALIZE);
  glPopMatrix();
  PARENT::PostDraw(rd);
}

