// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGlShader_GL_Rnr.h"
#include <Eye/Eye.h>

#include <GL/glew.h>

#define PARENT ZGlass_GL_Rnr

//______________________________________________________________________________
//
// This class is never rendered/drawn. Functions 
//   GLuint AssertShader() and
//   GLuint Compile()
// are called from ZGlProgram_GL_Rnr when needed.
//
// Need compilation failed flag - so that we don't retry this in render loop.
// Clear this on recompile ray.

//==============================================================================

void ZGlShader_GL_Rnr::_init()
{
  bRecompile = false;
  mShaderID = 0;
}

ZGlShader_GL_Rnr::ZGlShader_GL_Rnr(ZGlShader* idol) :
  ZGlass_GL_Rnr(idol),
  mZGlShader(idol)
{
  _init();
}

ZGlShader_GL_Rnr::~ZGlShader_GL_Rnr()
{}

//==============================================================================

void ZGlShader_GL_Rnr::AbsorbRay(Ray& ray)
{
  // Only handle ZGlShader::PRQN_recompile.

  using namespace RayNS;

  if(ray.fFID == ZGlShader::FID())
  {
    switch (ray.fRQN)
    {
      case ZGlShader::PRQN_recompile:
	cout << "Recompile " << mZGlShader->Identify() << endl;
	bRecompile = true;
	return;
      default:
	break;
    }
  }
  PARENT::AbsorbRay(ray);
}

//==============================================================================

GLuint ZGlShader_GL_Rnr::AssertShader()
{
  if (mShaderID && !bRecompile)
    return mShaderID;
  else
    return Compile();
}

GLuint ZGlShader_GL_Rnr::Compile()
{
  bRecompile = false;

  if (mShaderID == 0)
  {
    mShaderID = glCreateShader(mZGlShader->mType);
    if (mShaderID == 0)
    {
      cout << "Failed creating shader " << mZGlShader->Identify() << endl;
      return 0;
    }
  }

  {
    GMutexHolder prog_lock(mZGlShader->mProgMutex);
    const char* program =  mZGlShader->mProgram.Data();
    glShaderSource(mShaderID, 1, &program, 0);
  }

  glCompileShader(mShaderID);

  GLint status;
  glGetShaderiv(mShaderID, GL_COMPILE_STATUS, &status);
  auto_ptr<ZMIR> s_mir(mZGlShader->S_SetCompiled(status == GL_TRUE));
  fImg->fEye->Send(*s_mir);

  GLint log_len;
  glGetShaderiv(mShaderID, GL_INFO_LOG_LENGTH, &log_len);
  GLchar *log = new GLchar[log_len];
  glGetShaderInfoLog(mShaderID, log_len, 0, log);
  auto_ptr<ZMIR> l_mir(mZGlShader->S_SetLog(log));
  fImg->fEye->Send(*l_mir);

  cout << "Compilation log for " << mZGlShader->Identify() << endl << log;

  if (status == GL_FALSE)
  {
    glDeleteShader(mShaderID);
    mShaderID = 0;
  }

  return mShaderID;
}
