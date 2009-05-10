// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "LandMark_GL_Rnr.h"
#include <Glasses/Tringula.h>
#include <Rnr/GL/GLRnrDriver.h>
#include <Rnr/GL/TringTvor_GL_Rnr.h>

#include <GL/glew.h>

#define PARENT Extendio_GL_Rnr

/**************************************************************************/

void LandMark_GL_Rnr::_init()
{}

/**************************************************************************/

//void LandMark_GL_Rnr::PreDraw(RnrDriver* rd){}

//void LandMark_GL_Rnr::Draw(RnrDriver* rd){}

void LandMark_GL_Rnr::Render(RnrDriver* rd)
{
  LandMark &LM = * mLandMark;

  glPushAttrib(GL_ENABLE_BIT);

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  rd->GL()->Color(LM.mColor);

  TringTvor_GL_Rnr::Render(LM.GetMesh()->GetTTvor(), false);

  if (LM.bRnrIdealLine)
  {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glLineWidth(1);
    glBegin(GL_LINES);
    glVertex3f(LM.mSx, 0, LM.mLineHeight);
    Float_t d = TMath::TwoPi() / LM.mNLineSegments;
    Float_t s = 0;
    for (Int_t i = 1; i < LM.mNLineSegments; ++i)
    {
      s += d;
      glVertex3f(LM.mSx*cosf(s), LM.mSy*sinf(s), LM.mLineHeight);
    }
    glEnd();
  }

  glPopAttrib();

  TriMesh *TMesh = LM.mTringula->GetMesh();

  // This does no rendering - calculates line-line in global coordinates.
  // It is drawn in PostDraw().
  // In principle, this could be calculated in some LandMark function,
  // if we ever need it there as well.
  if (LM.bDetailedLine)
  {
    mLandLinePoints.clear();
    mLandLinePoints.reserve(1.2*LM.mNLineSegments);
    HTransF& t = LM.mTrans;
    Opcode::Point xvec(t.ref_base_vec_x()); xvec *= LM.mSx;
    Opcode::Point yvec(t.ref_base_vec_y()); yvec *= LM.mSy;

    UInt_t tidx = OPC_INVALID_ID;
    {
      mLandLinePoints.push_back(Opcode::Point());
      Opcode::Point& xx = mLandLinePoints.back();
      xx.Add(t.ref_pos(), xvec);
      TMesh->FindPointFromXYZH(xx, LM.mLineHeight, xx, 0, &tidx);
    }

    Float_t d = TMath::TwoPi() / LM.mNLineSegments;
    Float_t s = 0;
    for (Int_t i = 1; i <= LM.mNLineSegments; ++i)
    {
      s += d;
      Opcode::Point p;
      p.Mac2(t.ref_pos(), xvec, cosf(s), yvec, sinf(s));
      UInt_t lidx = tidx;
      TMesh->FindPointFromXYZH(p, LM.mLineHeight, p, 0, &tidx);
      if (tidx != lidx)
      {
	Opcode::Point delta;
	delta.Sub(p, mLandLinePoints.back());

	Float_t prev_dsqr = delta.SquareMagnitude();
	Opcode::Point x;
	do
	{
	  if (TMesh->FindTriangleExitPoint(lidx, mLandLinePoints.back(), delta, x, &lidx))
	  {
	    Opcode::Point now_delta; now_delta.Sub(p, x);
	    Float_t now_dsqr = now_delta.SquareMagnitude();
	    if ((delta | now_delta) > 0 && now_dsqr < prev_dsqr) {
	      mLandLinePoints.push_back(x);
	      prev_dsqr = now_dsqr;
	    } else {
	      break;
	    }
	  }
	  else
	  {
	    // Here, under some exit conditions (which are not known as return is only bool),
	    // we could search for the neigbouring triangles from the closest vertex - in proper
	    // direction - i think this could be calculated.
	    //
	    // Some magick is now already done in FindTriangleExitPoint().
	    break;
	  }
	} while (lidx != tidx);
      }
      mLandLinePoints.push_back(p);
    }
  }
  else
  {
    mLandLinePoints.resize(LM.mNLineSegments);
    HTransF& t = LM.mTrans;
    Opcode::Point xvec(t.ref_base_vec_x()); xvec *= LM.mSx;
    Opcode::Point yvec(t.ref_base_vec_y()); yvec *= LM.mSy;
    UInt_t  tidx = OPC_INVALID_ID;
    Float_t d = TMath::TwoPi() / LM.mNLineSegments;
    Float_t s = 0;
    for (Int_t i = 0; i < LM.mNLineSegments; ++i)
    {
      s += d;
      Opcode::Point &p = mLandLinePoints[i];
      p.Mac2(t.ref_pos(), xvec, cosf(s), yvec, sinf(s));
      LM.mTringula->GetMesh()->FindPointFromXYZH(p, LM.mLineHeight, p, 0, &tidx);
    }
  }
}

void LandMark_GL_Rnr::PostDraw(RnrDriver* rd)
{
  PARENT::PostDraw(rd);

  glLineWidth(2);
  rd->GL()->Color(mLandMark->mColor);
  GL_Capability_Switch loff(GL_LIGHTING, false);
  glBegin(GL_LINE_LOOP);
  std::vector<Opcode::Point>::iterator i = mLandLinePoints.begin();
  while (i != mLandLinePoints.end())
  {
    glVertex3fv(*i);
    ++i;
  }
  glEnd();
}
