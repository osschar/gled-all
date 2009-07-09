// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGlProgram_GL_Rnr.h"
#include "ZGlShader_GL_Rnr.h"
#include <Eye/Eye.h>
#include <RnrBase/RnrDriver.h>

#include <GL/glew.h>

#define PARENT ZGlass_GL_Rnr

//==============================================================================

void ZGlProgram_GL_Rnr::_init()
{
  bRelink      = false;
  bRecompile   = false;
  bBuildFailed = false;

  mProgID = 0;
}

ZGlProgram_GL_Rnr::ZGlProgram_GL_Rnr(ZGlProgram* idol) :
  ZGlass_GL_Rnr(idol),
  mZGlProgram(idol)
{
  _init();
}

ZGlProgram_GL_Rnr::~ZGlProgram_GL_Rnr()
{
  if (mProgID)
  {
    glDeleteProgram(mProgID);
  }
}

//==============================================================================

void ZGlProgram_GL_Rnr::AbsorbRay(Ray& ray)
{
  // Only handle ZGlProgram::PRQN_relink.
  // Could in principle also trigger on any list change, but
  // this might be premature.

  using namespace RayNS;

  if(ray.fFID == ZGlProgram::FID())
  {
    switch (ray.fRQN)
    {
      case ZGlProgram::PRQN_relink:
	cout << "Relink " << mZGlProgram->Identify() << endl;
	bRelink      = true;
	bBuildFailed = false;
	return;
      case ZGlProgram::PRQN_rebuild:
	cout << "Rebuild " << mZGlProgram->Identify() << endl;
	bRelink      = true;
	bRecompile   = true;
	bBuildFailed = false;
	return;
      default:
	break;
    }
  }
  PARENT::AbsorbRay(ray);
}

//==============================================================================

void ZGlProgram_GL_Rnr::PreDraw(RnrDriver* rd)
{
  if (bBuildFailed)
  {
    return;
  }

  if (bRelink && mProgID)
  {
    glDeleteProgram(mProgID);
    mProgID = 0;
  }

  if (mProgID == 0)
  {
    bBuildFailed = true;

    mProgID = glCreateProgram();
    if (mProgID == 0)
    {
      cout << "Failed creating program " << mZGlProgram->Identify() << endl;
      return;
    }

    {
      GMutexHolder shdr_lock(mZGlProgram->RefListMutex());
      AList::Stepper<> s(mZGlProgram);
      while(s.step())
      {
	ZGlShader_GL_Rnr* srnr = static_cast<ZGlShader_GL_Rnr*>(rd->GetLensRnr(*s));
	GLuint sid = bRecompile ? srnr->Compile() : srnr->AssertShader();
	if (sid == 0)
	{
	  cout << "Failed getting shader id for " << mZGlProgram->Identify() << endl;
	  glDeleteProgram(mProgID);
	  mProgID = 0;
	  return;
	}
	glAttachShader(mProgID, sid);
      }
      bRecompile = false;
    }

    glLinkProgram(mProgID);
    bRelink = false;

    GLint status;
    glGetProgramiv(mProgID, GL_LINK_STATUS, &status);
    auto_ptr<ZMIR> s_mir(mZGlProgram->S_SetLinked(status == GL_TRUE));
    fImg->fEye->Send(*s_mir);

    GLint log_len;
    glGetProgramiv(mProgID, GL_INFO_LOG_LENGTH, &log_len);
    GLchar *log = new GLchar[log_len];
    glGetProgramInfoLog(mProgID, log_len, 0, log);
    check_gl_error("get log");
    auto_ptr<ZMIR> l_mir(mZGlProgram->S_SetLog(log));
    fImg->fEye->Send(*l_mir);

    cout << "Link log for " << mZGlProgram->Identify() << endl << log;

    if (status == GL_FALSE)
    {
      glDeleteProgram(mProgID);
      mProgID = 0;
    } else {
      bBuildFailed = false;
    }
  }

  if (mProgID)
  {
    // PUSH ?
    glUseProgram(mProgID);
  }
}

void ZGlProgram_GL_Rnr::Draw(RnrDriver* rd)
{
  printf("draw program - should not be called\n");
}

void ZGlProgram_GL_Rnr::PostDraw(RnrDriver* rd)
{
  if (mProgID)
  {
    glUseProgram(0); // POP instead?
  }
  GLenum gl_err = glGetError();
  if(gl_err) {
    printf("prog-post GL error: %s.\n", gluErrorString(gl_err));
  }

}
