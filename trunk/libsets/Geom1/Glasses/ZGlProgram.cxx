// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGlProgram.h"
#include "ZGlProgram.c7"
#include "ZGlShader.h"
#include <Stones/SGlUniform.h>

// ZGlProgram

//______________________________________________________________________________
//
// Representation of a GLSL program.
// Children are enforced to be ZGLShader instances.
//
// ZGlProgram_GL_Rnr compiles all the shaders and links the program.
//
// Unifrom variables are extracted and information is stored in the local
// map. Defaults are parsed out of the shader source and stored as string.
//   uniform vec3  BrickColor;  // (1.0, 0.3, 0.2)
// If bSetUniDefaults is set (default), these values are assigned to
// appropriate GL uniform variables when the program is activated.
// See also SGlUniform and SGlUniform_GL_Rnr.
//
//
// TODO:
// 1. GL_Rnr::Draw() does nothing - can only be via state-modifier link.
// 2. Add class for holding alternate uniform variable values.
//    One could have a ZList (or ZNameMap) and use name, title of elements
//    for variable name, value string - but that is burning memory.
//    Better to have glass ZGlUniformValues that holds the lot in a list
//    and marks in the GL_Rnr which are really available.
//    ZGlProgram would have to become ZGlRnrMod for that, to allow for
//    optimized detection of program state via time-stamps.
//    Special Fltk class would have to be written.
//    In any case, the univorm-values could be populated from a succesfully
//    compiled program - all information is already available in mUniMap.
// 3. Why is ZImage not a RnrMod? Should we separate ZTexture and ZImage?
//    This might become relevant for sampler uniform varaibles.

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
