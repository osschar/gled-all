// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZImage_GL_Rnr.h"
#include <FL/gl.h>

/**************************************************************************/

void ZImage_GL_Rnr::_init()
{
  mStampTexture = 0;
  mTexture = 0;
}

ZImage_GL_Rnr::~ZImage_GL_Rnr()
{
  if(mTexture) glDeleteTextures(1, &mTexture);
}

/**************************************************************************/

void ZImage_GL_Rnr::init_texture()
{
  if(mImage->bLoaded == true) {
    if(mTexture == 0) {
      glGenTextures(1, &mTexture);
      //cout << "ZImage_GL_Rnr::init_texture for "<< mGlass->GetName() <<
      //" TexName " << mTexture << endl;
    }

    // load texture ... silly defaults ... work to do.
    // note that loading anything that is not 24bit RGB will crash gled
    // also ... should do resizing ... or sth ...

    glBindTexture(GL_TEXTURE_2D, mTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    ZImage::sILMutex.Lock();
    mImage->bind();
    glTexImage2D(GL_TEXTURE_2D, 0, 3, mImage->mW, mImage->mH, 0,
		 mImage->gl_format(), mImage->gl_type(), mImage->data());
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
  glEnable(GL_TEXTURE_2D);
  if(mImage->mStampReqTexture > mStampTexture) {
    init_texture();
    mStampTexture = mImage->mStampReqTexture;
  }
  if(mTexture) {
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); 
  }
}

void ZImage_GL_Rnr::Draw(RnrDriver* rd)
{
}

void ZImage_GL_Rnr::PostDraw(RnrDriver* rd)
{
  glPopAttrib();
  ZGlass_GL_Rnr::PostDraw(rd);
}
