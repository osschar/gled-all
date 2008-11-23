// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZNodeLink_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>

#include <GL/gl.h>

namespace OS   = OptoStructs;

#define PARENT ZNode_GL_Rnr

/**************************************************************************/

void ZNodeLink_GL_Rnr::_init()
{
  _setup_lens();
}

void ZNodeLink_GL_Rnr::_setup_lens()
{
  if(fImg) {
    OS::lZLinkDatum_i ld = fImg->fLinkData.begin();
    while(ld != fImg->fLinkData.end()) {
      if(&ld->GetLinkRef() == mZNodeLink->mLens.ptr_link()) {
	mLensLD = &(*ld);
	return;
      }
      ++ld;
    }
  }
  mLensLD = 0;
}

/**************************************************************************/

void ZNodeLink_GL_Rnr::SetImg(OS::ZGlassImg* newimg)
{
  PARENT::SetImg(newimg);
  _setup_lens();
}

/**************************************************************************/

void ZNodeLink_GL_Rnr::CreateRnrScheme(RnrDriver* rd)
{
  if(mLensLD && mLensLD->GetToGlass()) {
    A_Rnr* lens_rnr = rd->GetRnr(mLensLD->GetToImg());
    lens_rnr->crs_links(rd, mRnrScheme);
  }
  PARENT::CreateRnrScheme(rd);
}

/**************************************************************************/

// Pre/Post Draw to also push/pop the name of the Lens.

void ZNodeLink_GL_Rnr::PreDraw(RnrDriver* rd)
{
  PARENT::PreDraw(rd);
  if(mLensLD && mLensLD->GetToGlass()) {
    ZGlass_GL_Rnr* lens_rnr = (ZGlass_GL_Rnr*)(rd->GetRnr(mLensLD->GetToImg()));
    lens_rnr->ZGlass_GL_Rnr::PreDraw(rd);
  }
}

void ZNodeLink_GL_Rnr::PostDraw(RnrDriver* rd)
{
  if(mLensLD && mLensLD->GetToGlass()) {
    ZGlass_GL_Rnr* lens_rnr = (ZGlass_GL_Rnr*)(rd->GetRnr(mLensLD->GetToImg()));
    lens_rnr->ZGlass_GL_Rnr::PostDraw(rd);
  }
  PARENT::PostDraw(rd);
}
