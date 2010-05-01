// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGlass_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>
#include <Rnr/GL/SphereTrings.h>
#include <Ephra/Saturn.h>


//==============================================================================

ZGlass_GL_Rnr::ZGlass_GL_Rnr(ZGlass* d) : A_Rnr(), mGlass(d)
{
  mStampTring = 0;
  mDispList   = glGenLists(1);
  bRebuildDL         = true;

  bUsesSubPicking    = false;
  bSuppressNameLabel = false;
}

ZGlass_GL_Rnr::~ZGlass_GL_Rnr()
{
  glDeleteLists(mDispList, 1);
}


//==============================================================================
// Protected methods
//==============================================================================

void ZGlass_GL_Rnr::obtain_rnrmod(RnrDriver* rd, RnrModStore& rms)
{
  RnrMod* rm = rd->GetRnrMod(rms.fFid);
  if (rm != rms.fRnrMod || rm->fTringTS > rms.fTringTS)
  {
    bRebuildDL   = true;
    rms.fRnrMod  = rm;
    rms.fTringTS = rm->fTringTS;
  }
}

bool ZGlass_GL_Rnr::check_gl_error(const TString& msg)
{
  // Checks if GL error has occured.
  // Returns true if it did.

  GLenum gl_err = glGetError();
  if (gl_err)
  {
    cout << "GL error in " << mGlass->Identify() << ": " << msg << endl
	 << "  " << gluErrorString(gl_err) << endl;
    return true;
  }
  return false;
}


//==============================================================================
// Draw commands and its specializations.
//==============================================================================

void ZGlass_GL_Rnr::PreDraw(RnrDriver* rd)
{
  if (mGlass->bUseNameStack) rd->GL()->PushName(this);

  if (mGlass->mStampReqTring > mStampTring)
  {
    Triangulate(rd);
    mStampTring = mGlass->mTimeStamp;
    bRebuildDL  = true;
  }
}

void ZGlass_GL_Rnr::Draw(RnrDriver* rd)
{
  // cout <<"ZGlass_GL_Rnr::Draw rendering '"<< mGlass->GetName() <<"'.\n";

  if (mGlass->bUseDispList)
  {
    GLRnrDriver* glrd = rd->GL();
    if (bUsesSubPicking && glrd->PickingP())
    {
      Render(rd);
    }
    else if (bRebuildDL)
    {
      if (glrd->GetInDLRebuild())
      {
	Render(rd);
      }
      else
      {
	glrd->SetInDLRebuild(true);
        // Compared with GL_COMPILE and then glCallList() but there was
        // no difference. 24.2.07, gl-version ATI 2.0.6234
	glNewList(mDispList, GL_COMPILE_AND_EXECUTE);
	Render(rd);
	glEndList();
	glrd->SetInDLRebuild(false);
	bRebuildDL = false;
      }
    }
    else
    {
      glCallList(mDispList);
    }
  }
  else
  {
    Render(rd);
  }
}

void ZGlass_GL_Rnr::PostDraw(RnrDriver* rd)
{
  if (mGlass->bUseNameStack) rd->GL()->PopName();
}

//------------------------------------------------------------------------------

void ZGlass_GL_Rnr::Render(RnrDriver* rd)
{}

//------------------------------------------------------------------------------

void ZGlass_GL_Rnr::Redraw(RnrDriver* rd)
{
  bRebuildDL = true;
  rd->GL()->SetRedraw(true);
}


//==============================================================================
// Event handling
//==============================================================================

int ZGlass_GL_Rnr::Handle(RnrDriver* rd, Fl_Event& ev)
{
  // Check if the lens is a list and calls forwards Handle() to
  // list elements.
  // Returns 1 when one of the elements returns 1, otherwise 0.
  //
  // This is implemented here to avoid AList_GL_Rnr -- as list rendering
  // is already implemented in base classes.

  if (fImg->IsList())
  {
    namespace OS = OptoStructs;

    OS::lpZGlassImg_t *imgs = fImg->GetElementImgs();
    for (OS::lpZGlassImg_i i = imgs->begin(); i != imgs->end(); ++i)
    {
      A_Rnr *rnr = rd->GetRnr(*i);
      if (rnr->Handle(rd, ev))
	return 1;
    }
  }
  return 0;
}
