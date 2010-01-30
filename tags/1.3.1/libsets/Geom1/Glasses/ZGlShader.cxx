// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGlShader.h"
#include "ZGlShader.c7"

#include <Glasses/ZQueen.h>

#include <GL/glew.h>

#include <fstream>

// ZGlShader

//______________________________________________________________________________
//
//

ClassImp(ZGlShader);

//==============================================================================

void ZGlShader::_init()
{
  mType = 0;
  bAutoRecompile = true;

  bCompiled = false;
}

ZGlShader::ZGlShader(const Text_t* n, const Text_t* t) :
  ZGlass(n, t)
{
  _init();
}

ZGlShader::~ZGlShader()
{}

//==============================================================================

void ZGlShader::SetProgram(const Text_t* s)
{
  mProgram = s;
  Stamp(FID());
  if (bAutoRecompile)
    EmitRecompileRay();
}

void ZGlShader::EmitRecompileRay()
{
  if (mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_recompile, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}

//==============================================================================

void ZGlShader::Load(const Text_t* file)
{
  if (file != 0)
  {
    mFile = file;
    if (mFile.EndsWith(".vert"))
      mType = GL_VERTEX_SHADER;
    else if (mFile.EndsWith(".frag"))
      mType = GL_FRAGMENT_SHADER;
    else
      mType = 0;
  }

  {
    GMutexHolder prog_lock(mProgMutex);
    ifstream f(mFile);
    mProgram.ReadFile(f);
    f.close();
  }

  Stamp(FID());
}

void ZGlShader::Save(const Text_t* file)
{
  ofstream f(mFile);
  f << mProgram;
  f.close();
}

//==============================================================================

TString ZGlShader::type_as_string()
{
  switch (mType)
  {
    case GL_VERTEX_SHADER:   return "Vertex";
    case GL_FRAGMENT_SHADER: return "Fragment";
    default:                 return "Undef";
  }
}

void ZGlShader::PrintProgram()
{
  cout << type_as_string() << " program for " << Identify() << "\n" << mProgram;
}

void ZGlShader::PrintLog()
{
  cout << "Compile log for " << Identify() << "\n" << mLog;
}
