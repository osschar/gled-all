// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SGlUniform_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>

#include <TPRegexp.h>

#include <GL/glew.h>
TPMERegexp *SGlUniform_GL_Rnr::s_valuesep_re = 0;

//==============================================================================

SGlUniform_GL_Rnr::SGlUniform_GL_Rnr(SGlUniform* u) :
  mUni(u)
{
  mData.fInt = 0;

  if (s_valuesep_re == 0)
    s_valuesep_re = new TPMERegexp("[\\s,]+", "o");
}

SGlUniform_GL_Rnr::SGlUniform_GL_Rnr(const SGlUniform_GL_Rnr &a) :
  mUni (a.mUni)
{
  mData.fInt = 0;
}

SGlUniform_GL_Rnr::~SGlUniform_GL_Rnr()
{
  dealloc();
}

//==============================================================================

void SGlUniform_GL_Rnr::alloc()
{
  if (mUni->fIsFloat)
    mData.fFloat = new Float_t[mUni->fVarSize * mUni->fArrSize];
  else
    mData.fInt   = new Int_t  [mUni->fVarSize * mUni->fArrSize];
}

void SGlUniform_GL_Rnr::dealloc()
{
  if (mData.fInt == 0)
    return;

  if (mUni->fIsFloat)
    delete [] mData.fFloat;
  else
    delete [] mData.fInt;
}

void SGlUniform_GL_Rnr::realloc()
{
  dealloc();
  alloc();
}

//==============================================================================

void SGlUniform_GL_Rnr::parse(const TString& vals)
{
  realloc();

  Int_t nm = s_valuesep_re->Split(vals);
  if (nm != mUni->fVarSize * mUni->fArrSize)
  {
    printf("Bloody hell, not right number of fields: nm = %d, required = %d.\n",
	   nm, mUni->fVarSize * mUni->fArrSize);
    if (nm > mUni->fVarSize * mUni->fArrSize)
      nm = mUni->fVarSize * mUni->fArrSize;
  }
  for (Int_t i = 0; i < nm; ++i)
  {
    if (mUni->fIsFloat)
      mData.fFloat[i] = atof((*s_valuesep_re)[i]);
    else
      mData.fInt  [i] = atoi((*s_valuesep_re)[i]);
  }
}

void SGlUniform_GL_Rnr::parse_defaults()
{
  parse(mUni->fDefaults);
}

//==============================================================================

void SGlUniform_GL_Rnr::apply()
{
  switch (mUni->fType)
  {
    case GL_FLOAT:      glUniform1fv(mUni->fLocation, mUni->fArrSize, mData.fFloat); break;
    case GL_FLOAT_VEC2: glUniform2fv(mUni->fLocation, mUni->fArrSize, mData.fFloat); break;
    case GL_FLOAT_VEC3: glUniform3fv(mUni->fLocation, mUni->fArrSize, mData.fFloat); break;
    case GL_FLOAT_VEC4: glUniform4fv(mUni->fLocation, mUni->fArrSize, mData.fFloat); break;
    case GL_INT:        glUniform1iv(mUni->fLocation, mUni->fArrSize, mData.fInt);   break;
    case GL_INT_VEC2:   glUniform2iv(mUni->fLocation, mUni->fArrSize, mData.fInt);   break;
    case GL_INT_VEC3:   glUniform3iv(mUni->fLocation, mUni->fArrSize, mData.fInt);   break;
    case GL_INT_VEC4:   glUniform4iv(mUni->fLocation, mUni->fArrSize, mData.fInt);   break;
    case GL_BOOL:       glUniform1iv(mUni->fLocation, mUni->fArrSize, mData.fInt);   break;
    case GL_BOOL_VEC2:  glUniform2iv(mUni->fLocation, mUni->fArrSize, mData.fInt);   break;
    case GL_BOOL_VEC3:  glUniform3iv(mUni->fLocation, mUni->fArrSize, mData.fInt);   break;
    case GL_BOOL_VEC4:  glUniform4iv(mUni->fLocation, mUni->fArrSize, mData.fInt);   break;

    case GL_FLOAT_MAT2: glUniformMatrix2fv(mUni->fLocation, mUni->fArrSize, false, mData.fFloat); break;
    case GL_FLOAT_MAT3: glUniformMatrix3fv(mUni->fLocation, mUni->fArrSize, false, mData.fFloat); break;
    case GL_FLOAT_MAT4: glUniformMatrix4fv(mUni->fLocation, mUni->fArrSize, false, mData.fFloat); break;

    case GL_SAMPLER_1D:        glUniform1iv(mUni->fLocation, mUni->fArrSize, mData.fInt);    break;
    case GL_SAMPLER_2D:        glUniform1iv(mUni->fLocation, mUni->fArrSize, mData.fInt);    break;
    case GL_SAMPLER_3D:        glUniform1iv(mUni->fLocation, mUni->fArrSize, mData.fInt);    break;
    case GL_SAMPLER_CUBE:      glUniform1iv(mUni->fLocation, mUni->fArrSize, mData.fInt);    break;
    case GL_SAMPLER_1D_SHADOW: glUniform1iv(mUni->fLocation, mUni->fArrSize, mData.fInt);    break;
    case GL_SAMPLER_2D_SHADOW: glUniform1iv(mUni->fLocation, mUni->fArrSize, mData.fInt);    break;

    default: throw Exc_t("Jebo te uniform - unknown type\n");
  }  
}

//==============================================================================

const char* SGlUniform_GL_Rnr::unitype_to_name(GLenum t)
{
  switch (t)
  {
    case GL_FLOAT:      return "FLOAT";
    case GL_FLOAT_VEC2: return "FLOAT_VEC2";
    case GL_FLOAT_VEC3: return "FLOAT_VEC3";
    case GL_FLOAT_VEC4: return "FLOAT_VEC4";
    case GL_INT:        return "INT";
    case GL_INT_VEC2:   return "INT_VEC2";
    case GL_INT_VEC3:   return "INT_VEC3";
    case GL_INT_VEC4:   return "INT_VEC4";
    case GL_BOOL:       return "BOOL";
    case GL_BOOL_VEC2:  return "BOOL_VEC2";
    case GL_BOOL_VEC3:  return "BOOL_VEC3";
    case GL_BOOL_VEC4:  return "BOOL_VEC4";

    case GL_FLOAT_MAT2: return "FLOAT_MAT2";
    case GL_FLOAT_MAT3: return "FLOAT_MAT3";
    case GL_FLOAT_MAT4: return "FLOAT_MAT4";

    case GL_SAMPLER_1D:        return "SAMPLER_1D";
    case GL_SAMPLER_2D:        return "SAMPLER_2D";
    case GL_SAMPLER_3D:        return "SAMPLER_3D";
    case GL_SAMPLER_CUBE:      return "SAMPLER_CUBE";
    case GL_SAMPLER_1D_SHADOW: return "SAMPLER_1D_SHADOW";
    case GL_SAMPLER_2D_SHADOW: return "SAMPLER_2D_SHADOW";

    default: return "unknown";
  }
}

Int_t SGlUniform_GL_Rnr::unitype_to_size(GLenum t)
{
  switch (t)
  {
    case GL_FLOAT:      return 1;
    case GL_FLOAT_VEC2: return 2;
    case GL_FLOAT_VEC3: return 3;
    case GL_FLOAT_VEC4: return 4;
    case GL_INT:        return 1;
    case GL_INT_VEC2:   return 2;
    case GL_INT_VEC3:   return 3;
    case GL_INT_VEC4:   return 4;
    case GL_BOOL:       return 1;
    case GL_BOOL_VEC2:  return 2;
    case GL_BOOL_VEC3:  return 3;
    case GL_BOOL_VEC4:  return 4;

    case GL_FLOAT_MAT2: return 4;
    case GL_FLOAT_MAT3: return 9;
    case GL_FLOAT_MAT4: return 16;

    case GL_SAMPLER_1D:        return 1;
    case GL_SAMPLER_2D:        return 1;
    case GL_SAMPLER_3D:        return 1;
    case GL_SAMPLER_CUBE:      return 1;
    case GL_SAMPLER_1D_SHADOW: return 1;
    case GL_SAMPLER_2D_SHADOW: return 1;

    default:     return -1;
  }
}

Bool_t SGlUniform_GL_Rnr::unitype_is_float(GLenum t)
{
  switch (t)
  {
    case GL_FLOAT:      return true;
    case GL_FLOAT_VEC2: return true;
    case GL_FLOAT_VEC3: return true;
    case GL_FLOAT_VEC4: return true;
    case GL_INT:        return false;
    case GL_INT_VEC2:   return false;
    case GL_INT_VEC3:   return false;
    case GL_INT_VEC4:   return false;
    case GL_BOOL:       return false;
    case GL_BOOL_VEC2:  return false;
    case GL_BOOL_VEC3:  return false;
    case GL_BOOL_VEC4:  return false;

    case GL_FLOAT_MAT2: return true;
    case GL_FLOAT_MAT3: return true;
    case GL_FLOAT_MAT4: return true;

    case GL_SAMPLER_1D:        return false;
    case GL_SAMPLER_2D:        return false;
    case GL_SAMPLER_3D:        return false;
    case GL_SAMPLER_CUBE:      return false;
    case GL_SAMPLER_1D_SHADOW: return false;
    case GL_SAMPLER_2D_SHADOW: return false;

    default: return false;
  }
}
