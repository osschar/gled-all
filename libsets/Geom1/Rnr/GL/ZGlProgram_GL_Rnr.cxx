// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGlProgram_GL_Rnr.h"
#include "ZGlShader_GL_Rnr.h"
#include <Eye/Eye.h>
#include <RnrBase/RnrDriver.h>

#include <Stones/SGlUniform.h>
#include "SGlUniform_GL_Rnr.h"

#include <TPRegexp.h>

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

void ZGlProgram_GL_Rnr::build_program(RnrDriver* rd)
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
    return;
  }

  bBuildFailed = false;

  extract_uniform_vars();
}

void ZGlProgram_GL_Rnr::extract_uniform_vars()
{
  ZGlProgram::mName2pUniform_t umap;

  mUniDefaults.clear();

  GLint n_uni, name_len;
  glGetProgramiv(mProgID, GL_ACTIVE_UNIFORMS, &n_uni);
  glGetProgramiv(mProgID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &name_len);
  GLchar* name = new GLchar[name_len];
  GLenum  type;
  GLint   arr_size;
  for (Int_t uni = 0; uni < n_uni; ++uni)
  {
    glGetActiveUniform(mProgID, uni, name_len, 0, &arr_size, &type, name);

    GLint loc = glGetUniformLocation(mProgID, name);
    // printf("Uniform %d: '%s'[%d] %s; loc=%d\n",
    //        uni, name, arr_size, SGlUniform_GL_Rnr::unitype_to_name(type), loc);

    // i R assuming the uni and loc are the same ... check anyway.
    if (loc != -1 && loc != uni)
      printf("Warning - unform sequential id and location do not match!\n");

    if (strncmp(name, "gl_", 3) != 0)
    {
      // Loop over shaders, grep 'uniform ' lines for default values
      TString defvals;
      {
	TString re("^\\s*uniform\\s+\\w+\\s+");
	re += name;
	re += "\\s*;\\s*//\\s*\\(\\s*([-\\d.,\\s]+\\S)\\s*\\)";
	TPMERegexp unire(re, "mo");

	GMutexHolder list_lock(mZGlProgram->RefListMutex());
	AList::Stepper<> s(mZGlProgram);
	while(s.step())
	{
	  ZGlShader   *shdr = static_cast<ZGlShader*>(*s);
	  GMutexHolder shdr_lock(shdr->mProgMutex);
	  Int_t nm = unire.Match(shdr->mProgram);
	  if (nm == 2)
	  {
	    defvals = unire[1];
	    // printf("Found match for uni-var %s defaults -- '%s'\n", name, defvals.Data());
	  }
	}
      }

      SGlUniform* u = new SGlUniform
	(name, defvals, SGlUniform_GL_Rnr::unitype_is_float(type), type,
	 SGlUniform_GL_Rnr::unitype_to_size(type),
	 arr_size, loc);
      umap.insert(make_pair(name, u));

      if (!defvals.IsNull())
      {
	mUniDefaults.push_back(SGlUniform_GL_Rnr(u));
	mUniDefaults.back().parse_defaults();
      }
    }
  }
  delete [] name;

  mZGlProgram->swap_unimap(umap);
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
    build_program(rd);
  }

  if (mProgID)
  {
    // PUSH ?
    glUseProgram(mProgID);

    if (mZGlProgram->bSetUniDefaults)
    {
      for (lUniGl_i i = mUniDefaults.begin(); i != mUniDefaults.end(); ++i)
      {
	i->apply();
      }
    }
  }

  check_gl_error("PreDraw()");
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
  check_gl_error("PostDraw()");
}
