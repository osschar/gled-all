// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZImage_GL_Rnr.h"
#include <Eye/Eye.h>
#include <Stones/ZMIR.h> // Required by gcc-3.2 (not by gcc-4)

#include <TMath.h>

/**************************************************************************/

Int_t ZImage_GL_Rnr::sRescaleToPow2 = -1;

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

void  ZImage_GL_Rnr::check_rescale()
{
  static const Exc_t _eh("ZImage_GL_Rnr::check_rescale ");

  if (sRescaleToPow2 == -1) {
    sRescaleToPow2 = 0;
    TString vendor((const char*) glGetString(GL_VENDOR));

    TString version((const char*) glGetString(GL_VERSION));
    version.Replace(version.First(' '), 1024, 0, 0);
    lStr_t vs; GledNS::split_string(version, vs, '.');
    Int_t major = 1, minor = 0, patch = 0;
    if(!vs.empty()) { major = atoi(vs.front().Data()); vs.pop_front(); }
    if(!vs.empty()) { minor = atoi(vs.front().Data()); vs.pop_front(); }
    if(!vs.empty()) { patch = atoi(vs.front().Data()); vs.pop_front(); }

    Exc_t mh = _eh + "enabling pow2 texture rescale: ";
    if (major < 1 || (major == 1 && minor < 4)) {
      ISmess(mh + "GL < 1.4.");
      sRescaleToPow2 = 1;
    }
    else if (vendor.BeginsWith("ATI")) {
      if (major <= 2 && minor <= 1 && patch <= 7659) {
        ISmess(mh + "ATI, GL <= 2.1.7659 (very slow rendering).");
        sRescaleToPow2 = 1;
      }
    }
  }

  if (sRescaleToPow2 == 1) {
    Int_t w = nearest_pow2(mImage->mW);
    Int_t h = nearest_pow2(mImage->mH);
    if (w != mImage->mW || h != mImage->mH) {
      ISmess(GForm("%srescaling '%s' (%d,%d) -> (%d,%d).", _eh.Data(),
                   mImage->GetName(), mImage->mW, mImage->mH, w, h));
      iluImageParameter(ILU_FILTER, ILU_BILINEAR);
      iluScale(w, h, 1);
      mImage->mW = w;
      mImage->mH = h;
      auto_ptr<ZMIR> m( mImage->S_UpdateAllViews() );
      fImg->fEye->Send(*m);
    }
  }
}

Int_t ZImage_GL_Rnr::nearest_pow2(Int_t d)
{
  Int_t np2 = TMath::Nint(TMath::Log2(d));
  return 1 << np2;
}

void ZImage_GL_Rnr::Triangulate(RnrDriver* rd)
{
  if (mImage->IsBindable())
  {
    if (mTexture == 0)
      glGenTextures(1, &mTexture);

    glBindTexture(GL_TEXTURE_2D, mTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mImage->mSWrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mImage->mTWrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mImage->mMagFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mImage->mMinFilter);

    ZImage::sILMutex.Lock();
    mImage->bind();
    check_rescale();
    glTexImage2D(GL_TEXTURE_2D, 0, mImage->mIntFmt, mImage->mW, mImage->mH, 0,
		 mImage->gl_format(), mImage->gl_type(), mImage->data());
    mImage->unbind();
    ZImage::sILMutex.Unlock();
  }
  else
  {
    if(mTexture)
    {
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
