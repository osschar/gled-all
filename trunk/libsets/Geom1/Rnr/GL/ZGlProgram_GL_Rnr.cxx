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
    //        uni, name, arr_size, unitype_to_name(type), loc);

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
	  if (nm == 2) {
	    defvals = unire[1];
	    // printf("Found match for uni-var %s defaults -- '%s'\n", name, defvals.Data());
	  }
	}
      }

      umap[name] = new SGlUniform(name, defvals, unitype_is_float(type), type, unitype_to_size(type), arr_size, loc);
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
      GMutexHolder uni_lock(mZGlProgram->mUniMutex);

      for (ZGlProgram::mName2pUniform_i i = mZGlProgram->mUniMap.begin(); i != mZGlProgram->mUniMap.end(); ++i)
      {
	SGlUniform_GL_Rnr urnr(i->second);
	if (!i->second->fDefaults.IsNull())
	{
	  urnr.parse_and_apply(i->second->fDefaults);
	}
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

const char* ZGlProgram_GL_Rnr::unitype_to_name(GLenum t)
{
  switch (t)
  {
    case 0x1406: return "FLOAT";
    case 0x8B50: return "FLOAT_VEC2";
    case 0x8B51: return "FLOAT_VEC3";
    case 0x8B52: return "FLOAT_VEC4";
    case 0x1404: return "INT";
    case 0x8B53: return "INT_VEC2";
    case 0x8B54: return "INT_VEC3";
    case 0x8B55: return "INT_VEC4";
    case 0x8B56: return "BOOL";
    case 0x8B57: return "BOOL_VEC2";
    case 0x8B58: return "BOOL_VEC3";
    case 0x8B59: return "BOOL_VEC4";
    case 0x8B5A: return "FLOAT_MAT2";
    case 0x8B5B: return "FLOAT_MAT3";
    case 0x8B5C: return "FLOAT_MAT4";
    case 0x8B5D: return "SAMPLER_1D";
    case 0x8B5E: return "SAMPLER_2D";
    case 0x8B5F: return "SAMPLER_3D";
    case 0x8B60: return "SAMPLER_CUBE";
    case 0x8B61: return "SAMPLER_1D_SHADOW";
    case 0x8B62: return "SAMPLER_2D_SHADOW";
    default:     return "unknown";
  }
}

Int_t ZGlProgram_GL_Rnr::unitype_to_size(GLenum t)
{
  switch (t)
  {
    case 0x1406: return 1;  // FLOAT
    case 0x8B50: return 2;
    case 0x8B51: return 3;
    case 0x8B52: return 4;
    case 0x1404: return 1;  // INT
    case 0x8B53: return 2;
    case 0x8B54: return 3;
    case 0x8B55: return 4;
    case 0x8B56: return 1;  // BOOL
    case 0x8B57: return 2;
    case 0x8B58: return 3;
    case 0x8B59: return 4;
    case 0x8B5A: return 4;  // FLOAT_MAT2;
    case 0x8B5B: return 9;  // FLOAT_MAT3;
    case 0x8B5C: return 16; // FLOAT_MAT4;
    case 0x8B5D: return 1;  // SAMPLER_1D;
    case 0x8B5E: return 1;  // SAMPLER_2D;
    case 0x8B5F: return 1;  // SAMPLER_3D;
    case 0x8B60: return 1;  // SAMPLER_CUBE;
    case 0x8B61: return 1;  // SAMPLER_1D_SHADOW;
    case 0x8B62: return 1;  // SAMPLER_2D_SHADOW;
    default:     return -1;
  }
}

Bool_t ZGlProgram_GL_Rnr::unitype_is_float(GLenum t)
{
  switch (t)
  {
    case 0x1406: return true;   // FLOAT
    case 0x8B50: return true;
    case 0x8B51: return true;
    case 0x8B52: return true;
    case 0x1404: return false;  // INT
    case 0x8B53: return false;
    case 0x8B54: return false;
    case 0x8B55: return false;
    case 0x8B56: return false;  // BOOL
    case 0x8B57: return false;
    case 0x8B58: return false;
    case 0x8B59: return false;
    case 0x8B5A: return true;   // FLOAT_MAT2;
    case 0x8B5B: return true;   // FLOAT_MAT3;
    case 0x8B5C: return true;   // FLOAT_MAT4;
    case 0x8B5D: return false;  // SAMPLER_1D;
    case 0x8B5E: return false;  // SAMPLER_2D;
    case 0x8B5F: return false;  // SAMPLER_3D;
    case 0x8B60: return false;  // SAMPLER_CUBE;
    case 0x8B61: return false;  // SAMPLER_1D_SHADOW;
    case 0x8B62: return false;  // SAMPLER_2D_SHADOW;
    default:     return false;
  }
}
