// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Tringula_GL_Rnr.h"

#include <Glasses/TriMesh.h>
#include <Rnr/GL/TringTvor_GL_Rnr.h>

#include <Opcode/Opcode.h>

#include <GL/glew.h>

#define PARENT ZNode_GL_Rnr

/**************************************************************************/

void Tringula_GL_Rnr::_init()
{
  mMeshTringStamp = 0;
}

Tringula_GL_Rnr::~Tringula_GL_Rnr()
{}

/******************************************************************************/

void Tringula_GL_Rnr::Draw(RnrDriver* rd)
{
  Tringula&   T = *mTringula;
  TringTvor* TT =  T.mMesh->GetTTvor();
  if (TT == 0) return;
  if (mMeshTringStamp < T.mMesh->GetStampReqTring())
  {
    bRebuildDL = true;
    mMeshTringStamp = T.mMesh->GetStampReqTring();
  }
  PARENT::Draw(rd);
}

void Tringula_GL_Rnr::Render(RnrDriver* rd)
{
  Tringula  &T  = *mTringula;
  TringTvor &TT = *T.mMesh->GetTTvor();
  glColor4fv(T.mColor());

  assert(TT.HasNorms());
  TringTvor_GL_Rnr::Render(&TT, true);
}
