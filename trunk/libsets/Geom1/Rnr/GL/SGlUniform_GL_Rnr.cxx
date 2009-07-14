// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SGlUniform_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>

#include <TPRegexp.h>

#include <GL/glew.h>

//==============================================================================

void SGlUniform_GL_Rnr::_init()
{}

SGlUniform_GL_Rnr::SGlUniform_GL_Rnr(SGlUniform* u) :
  mUni(u)
{
  _init();
}

SGlUniform_GL_Rnr::~SGlUniform_GL_Rnr()
{}

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
  if (mUni->fIsFloat)
    delete [] mData.fFloat;
  else
    delete [] mData.fInt;
}

void SGlUniform_GL_Rnr::parse_and_apply(const TString& vals)
{
  alloc();

  TPMERegexp sep("[\\s,]+");
  Int_t nm = sep.Split(vals);
  if (nm != mUni->fVarSize * mUni->fArrSize) {
    printf("Bloody hell, not right number of fields: nm = %d, required = %d.\n",
	   nm, mUni->fVarSize * mUni->fArrSize);
    if (nm > mUni->fVarSize * mUni->fArrSize)
      nm = mUni->fVarSize * mUni->fArrSize;
  }
  for (Int_t i = 0; i < nm; ++i)
  {
    if (mUni->fIsFloat)
      mData.fFloat[i] = atof(sep[i]);
    else
      mData.fInt  [i] = atoi(sep[i]);
  }
  switch (mUni->fType)
  {
    case 0x1406: glUniform1fv(mUni->fLocation, mUni->fArrSize, mData.fFloat); break;
    case 0x8B50: glUniform2fv(mUni->fLocation, mUni->fArrSize, mData.fFloat); break;
    case 0x8B51: glUniform3fv(mUni->fLocation, mUni->fArrSize, mData.fFloat); break;
    case 0x8B52: glUniform4fv(mUni->fLocation, mUni->fArrSize, mData.fFloat); break;
    case 0x1404: glUniform1iv(mUni->fLocation, mUni->fArrSize, mData.fInt);   break;
    case 0x8B53: glUniform2iv(mUni->fLocation, mUni->fArrSize, mData.fInt);   break;
    case 0x8B54: glUniform3iv(mUni->fLocation, mUni->fArrSize, mData.fInt);   break;
    case 0x8B55: glUniform4iv(mUni->fLocation, mUni->fArrSize, mData.fInt);   break;
    case 0x8B56: glUniform1iv(mUni->fLocation, mUni->fArrSize, mData.fInt);   break;
    case 0x8B57: glUniform2iv(mUni->fLocation, mUni->fArrSize, mData.fInt);   break;
    case 0x8B58: glUniform3iv(mUni->fLocation, mUni->fArrSize, mData.fInt);   break;
    case 0x8B59: glUniform4iv(mUni->fLocation, mUni->fArrSize, mData.fInt);   break;
    case 0x8B5A: glUniformMatrix2fv(mUni->fLocation, mUni->fArrSize, false, mData.fFloat); break;
    case 0x8B5B: glUniformMatrix3fv(mUni->fLocation, mUni->fArrSize, false, mData.fFloat); break;
    case 0x8B5C: glUniformMatrix4fv(mUni->fLocation, mUni->fArrSize, false, mData.fFloat); break;
    case 0x8B5D: glUniform1iv(mUni->fLocation, mUni->fArrSize, mData.fInt);    break;
    case 0x8B5E: glUniform1iv(mUni->fLocation, mUni->fArrSize, mData.fInt);    break;
    case 0x8B5F: glUniform1iv(mUni->fLocation, mUni->fArrSize, mData.fInt);    break;
    case 0x8B60: glUniform1iv(mUni->fLocation, mUni->fArrSize, mData.fInt);    break;
    case 0x8B61: glUniform1iv(mUni->fLocation, mUni->fArrSize, mData.fInt);    break;
    case 0x8B62: glUniform1iv(mUni->fLocation, mUni->fArrSize, mData.fInt);    break;
    default:     throw Exc_t("Jebo te uniform - unknown type\n");
  }

  dealloc();
}
