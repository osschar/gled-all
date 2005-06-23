// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZImage
//
// This is a glass wrapper for DevIL image.
// It should be used for loading/saving/binding of images, as this class
// contains a static mutex to perform locking of all IL operations.
//
// Some ILU image transformations are supported (blurring, contrast).
// Do NOT use shadowing when applying the transforms.

#include "ZImage.h"

#include <IL/il.h>
#include <IL/ilu.h>
#include <GL/gl.h>

namespace {
  /* IL reporting */

  bool il_err(const char* s) {
    ILenum er = ilGetError();
    if(er) {
      printf("DevIL error at %s error: %s\n", s, iluErrorString(er));
    }
    return (er != 0);
  }
  char* fmt2name(ILint fmt) {
    switch(fmt) {
    case IL_COLOR_INDEX: return "COLOR_INDEX";
    case IL_RGB: return "RGB";
    case IL_RGBA: return "RGBA";
    case IL_BGR: return "BGR";
    case IL_BGRA: return "BGRA";
    case IL_LUMINANCE: return "LUMINANCE";
    case IL_LUMINANCE_ALPHA: return "LUMINANCE_ALPHA";
    default: return "unknown";
    }
  }
  char* typ2name(ILint typ) {
    switch(typ) {
    case IL_BYTE: return "BYTE";
    case IL_UNSIGNED_BYTE: return "UNSIGNED_BYTE";
    case IL_SHORT: return "SHORT";
    case IL_UNSIGNED_SHORT: return "UNSIGNED_SHORT";
    case IL_INT: return "INT";
    case IL_UNSIGNED_INT: return "UNSIGNED_INT";
    case IL_FLOAT: return "FLOAT";
    case IL_DOUBLE: return "DOUBLE";
    default: return "unknown";
    }
  }
  void il_id() {
    // Display the image's dimensions to the user.
    ILint fmt, typ;
    printf("Width: %d  Height: %d  Depth: %d Bytespp: %d Bpp: %d\n",
	   ilGetInteger(IL_IMAGE_WIDTH),
	   ilGetInteger(IL_IMAGE_HEIGHT),
	   ilGetInteger(IL_IMAGE_DEPTH),
	   ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL),
	   ilGetInteger(IL_IMAGE_BITS_PER_PIXEL));

    fmt = ilGetInteger(IL_IMAGE_FORMAT);
    typ = ilGetInteger(IL_IMAGE_TYPE);

    printf("Format: %s  Type: %s\n",
	   fmt2name(fmt), typ2name(typ));
  }

}


ClassImp(ZImage)

GMutex ZImage::sILMutex(GMutex::recursive);
Bool_t ZImage::sVerboseLoad(false);

/**************************************************************************/

void ZImage::_init()
{
  mIL_Name = 0;
  mW = mH = 0;
  mImgFmt = 0;
  mImgTyp = 0;
  mIntFmt = GL_RGB8;

  mMagFilter = mMinFilter = GL_NEAREST;
  mEnvMode = GL_DECAL;

  bLoadAdEnlight = false;
  bUseShadowing  = true;
  bLoaded        = false;
  bShadowed      = false;
}

ZImage::~ZImage() {
  delete_image();
}

void ZImage::AdEnlightenment()
{
  ZGlass::AdEnlightenment();
  if(bLoadAdEnlight) Load();
}

/**************************************************************************/

void ZImage::SetUseShadowing(Bool_t useshadowing)
{
  if(useshadowing == bUseShadowing) return;
  if(useshadowing) {
    if(bLoaded && !bShadowed) {
      shadow();
    }
  } else {
    if(bLoaded && bShadowed) {
      bind();
    }
  }
  bUseShadowing = useshadowing;
  Stamp(FID());
}

void ZImage::Load()
{
  sILMutex.Lock();

  delete_image();
  bLoaded = false;
  ilGenImages(1, &mIL_Name);
  if(il_err("Gen images")) goto end;

  ilBindImage(mIL_Name);
  if(il_err("Bind Image")) goto end;

  if (!ilLoadImage(const_cast<char *>(mFile.Data()))) {
    il_err(GForm("Load Image <file:%s>", mFile.Data()));
    goto end;
  }

  if(sVerboseLoad) il_id();

  mW = ilGetInteger(IL_IMAGE_WIDTH);
  mH = ilGetInteger(IL_IMAGE_HEIGHT);
  mImgFmt = gl_format();
  mImgTyp = gl_type();
  bLoaded = true;

  if(bUseShadowing) shadow();

 end:
  mStampReqTring = Stamp(FID());
  sILMutex.Unlock();
}

void ZImage::Unload()
{
  sILMutex.Lock();
  delete_image();
  bLoaded = false; bShadowed = false;
  mStampReqTring = Stamp(FID());
  sILMutex.Unlock();
}

void ZImage::Save()
{
  // Could easily be implemented now ... that we have devil.
}

/**************************************************************************/

void ZImage::BlurAverage(UInt_t count)
{
  static const string _eh("ZImage::BlurAverage ");
  if(bUseShadowing) {
    warn_caller(_eh + "has no effect with shadowing on.");
    return;
  }

  sILMutex.Lock();
  bind();
  iluBlurAvg(count);
  unbind();
  sILMutex.Unlock();
  mStampReqTring = Stamp(FID());
}

void ZImage::BlurGaussian(UInt_t count)
{
  static const string _eh("ZImage::BlurGaussian ");
  if(bUseShadowing) {
    warn_caller(_eh + "has no effect with shadowing on.");
    return;
  }

  sILMutex.Lock();
  bind();
  iluBlurGaussian(count);
  unbind();
  sILMutex.Unlock();
  mStampReqTring = Stamp(FID());
}

void ZImage::Contrastify(Float_t contrast)
{
  static const string _eh("ZImage::Contrastify ");
  if(bUseShadowing) {
    warn_caller(_eh + "has no effect with shadowing on.");
    return;
  }

  if(contrast > 1.7)  contrast = 1.7;
  if(contrast < -0.5) contrast = -0.5;
  sILMutex.Lock();
  bind();
  iluContrast(contrast);
  unbind();
  sILMutex.Unlock();
  mStampReqTring = Stamp(FID());
}

void ZImage::Equalize()
{
  static const string _eh("ZImage::Equalize ");
  if(bUseShadowing) {
    warn_caller(_eh + "has no effect with shadowing on.");
    return;
  }

  sILMutex.Lock();
  bind();
  iluEqualize();
  unbind();
  sILMutex.Unlock();
  mStampReqTring = Stamp(FID());
}

/**************************************************************************/

// User's responsibility to lock/unlock image operations
// and assert that image is loaded.

void ZImage::bind()
{
  if(bShadowed && bUseShadowing) {

    ilGenImages(1, &mIL_Name);
    if(il_err("Gen images")) goto error;

    ilBindImage(mIL_Name);
    if(il_err("Bind Image")) goto error;

    if (!ilLoadImage(const_cast<char *>(mFile.Data()))) {
      il_err("Load Image");
      goto error;
    }

    bShadowed = false;

  } else {

    ilBindImage(mIL_Name);
    if(il_err("Bind Image")) goto error;

  }

  return;

 error:
  bLoaded = false;
  // Do not emit the stamp ...
}

void ZImage::unbind()
{
  if(bUseShadowing && !bShadowed) {
    shadow();
  }
}

void ZImage::shadow()
{
  delete_image();
  bShadowed = true;
}

void ZImage::delete_image()
{
  if(mIL_Name) {
    ilDeleteImages(1, &mIL_Name);
    mIL_Name = 0;
  }
}

/**************************************************************************/

int ZImage::w() {
  return ilGetInteger(IL_IMAGE_WIDTH);
}

int ZImage::h() {
  return ilGetInteger(IL_IMAGE_HEIGHT);
}

void* ZImage::data()
{
  return ilGetData();
}

int ZImage::gl_format()
{
  ILint fmt = ilGetInteger(IL_IMAGE_FORMAT);
  switch(fmt) {
  case IL_COLOR_INDEX:		return GL_COLOR_INDEX;
  case IL_RGB:			return GL_RGB;
  case IL_RGBA:			return GL_RGBA;
  case IL_BGR:			return GL_BGR;
  case IL_BGRA:			return GL_BGRA;
  case IL_LUMINANCE:		return GL_LUMINANCE;
  case IL_LUMINANCE_ALPHA:	return GL_LUMINANCE_ALPHA;
  default:			return 0;
  }

}

int ZImage::gl_type()
{
  ILint typ = ilGetInteger(IL_IMAGE_TYPE);
  switch(typ) {
  case IL_BYTE:			return GL_BYTE;
  case IL_UNSIGNED_BYTE:	return GL_UNSIGNED_BYTE;
  case IL_SHORT:		return GL_SHORT;
  case IL_UNSIGNED_SHORT:	return GL_UNSIGNED_SHORT;
  case IL_INT:			return GL_INT;
  case IL_UNSIGNED_INT:		return GL_UNSIGNED_INT;
  case IL_FLOAT:		return GL_FLOAT;
  case IL_DOUBLE:		return GL_DOUBLE;
  default: return 0;
  }
}

/**************************************************************************/

// For now, user init is here ... when another piece will need initialization
// it would better be placed somwhere else.

void libGeom1_GLED_user_init()
{
  ilInit();
  iluInit();
}

void *Geom1_GLED_user_init = (void*)libGeom1_GLED_user_init;

/**************************************************************************/

#include "ZImage.c7"
