// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGlProgram.h"
#include "ZGlProgram.c7"
#include "ZGlShader.h"

// ZGlProgram

//______________________________________________________________________________
//
//

ClassImp(ZGlProgram);

//==============================================================================

void ZGlProgram::_init()
{
  bSetUniDefaults = true;

  bLinked = false;
}

ZGlProgram::ZGlProgram(const Text_t* n, const Text_t* t) :
  ZList(n, t)
{
  _init();
  SetElementFID(ZGlShader::FID());
}

ZGlProgram::~ZGlProgram()
{}

//==============================================================================

void ZGlProgram::swap_unimap(mName2pUniform_t& umap)
{
  GMutexHolder uni_lock(mUniMutex);

  for (mName2pUniform_i i = mUniMap.begin(); i != mUniMap.end(); ++i)
  {
    i->second->DecRefCount();
  }
  mUniMap.swap(umap);
  for (mName2pUniform_i i = mUniMap.begin(); i != mUniMap.end(); ++i)
  {
    i->second->IncRefCount();
  }
}

//==============================================================================

void ZGlProgram::EmitRelinkRay()
{
  if (mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_relink, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}
void ZGlProgram::EmitRebuildRay()
{
  if (mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_rebuild, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}

void ZGlProgram::ReloadAndRebuild()
{
  // Running in a detached thread.

  lpZGlass_t shaders;
  CopyList(shaders);
  for (lpZGlass_i s = shaders.begin(); s != shaders.end(); ++s)
  {
    GLensWriteHolder slck(*s);
    static_cast<ZGlShader*>(*s)->Load();
  }
  GLensWriteHolder plck(this);
  EmitRebuildRay();
}

//==============================================================================

void ZGlProgram::PrintLog()
{
  cout << "Link log for " << Identify() << "\n" << mLog;
}

void ZGlProgram::PrintUniforms()
{
  for (mName2pUniform_i i = mUniMap.begin(); i != mUniMap.end(); ++i)
  {
    cout << i->first << "  " << i->second->fDefaults << endl;
  }
}
