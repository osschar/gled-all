// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZImage_GL_Rnr.h"

/**************************************************************************/

void ZImage_GL_Rnr::_init()
{
  mStampTexture = 0;
  mTexture = 0;
  // From ZGlass_GL_Rnr:
  bUseNameStack = false;
}

ZImage_GL_Rnr::~ZImage_GL_Rnr()
{
  if(mTexture) glDeleteTextures(1, &mTexture);
}

/**************************************************************************/

void ZImage_GL_Rnr::Triangulate(RnrDriver* rd)
{
  if(mImage->bLoaded == true) {
    if(mTexture == 0) {
      glGenTextures(1, &mTexture);
    }

    // load texture ... silly defaults ... work to do.
    // also ... should do resizing ... or sth ...

    glBindTexture(GL_TEXTURE_2D, mTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mImage->mMagFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mImage->mMinFilter);

    ZImage::sILMutex.Lock();
    mImage->bind();
    glTexImage2D(GL_TEXTURE_2D, 0, mImage->mIntFmt, mImage->mW, mImage->mH, 0,
		 mImage->gl_format(), mImage->gl_type(), mImage->data());
    mImage->unbind();
    ZImage::sILMutex.Unlock();

  } else {
    if(mTexture) {
      glDeleteTextures(1, &mTexture);
      mTexture = 0;
    }
  }
}

/**************************************************************************/

void ZImage_GL_Rnr::PreDraw(RnrDriver* rd)
{
  ZGlass_GL_Rnr::PreDraw(rd);
  glPushAttrib(GL_TEXTURE_BIT);
  if(mTexture) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mImage->mEnvMode); 
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, mImage->mEnvColor());
  } else {
    glDisable(GL_TEXTURE_2D);
  }
}

void ZImage_GL_Rnr::Draw(RnrDriver* rd)
{}

void ZImage_GL_Rnr::PostDraw(RnrDriver* rd)
{
  glPopAttrib();
  ZGlass_GL_Rnr::PostDraw(rd);
}
