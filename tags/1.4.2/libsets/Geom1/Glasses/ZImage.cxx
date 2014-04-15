// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
//
// This is a glass wrapper for DevIL image.
// It should be used for loading/saving/binding of images, as this class
// contains a static mutex to perform locking of all IL operations.
//
// Some ILU image transformations are not wrapped yet.
// Do NOT use shadowing when applying the transforms.
//
// When mMinFilter is one of mip-map ones, the mip-maps are generated
// automatically by calling glGenerateMipmapEXT().

#include "ZImage.h"
#include "ZImage.c7"

#include <IL/il.h>
#include <IL/ilu.h>
#include <GL/glew.h>

#include <TMath.h>
#include <TSystem.h>

namespace {
  /* IL reporting */

  bool il_err(const char* s) {
    ILenum er = ilGetError();
    if(er) {
      printf("DevIL error at %s error: %s\n", s, iluErrorString(er));
    }
    return (er != 0);
  }
  const char* fmt2name(ILint fmt) {
    switch(fmt) {
    case IL_COLOR_INDEX:     return "COLOR_INDEX";
    case IL_RGB:             return "RGB";
    case IL_RGBA:            return "RGBA";
    case IL_BGR:             return "BGR";
    case IL_BGRA:            return "BGRA";
    case IL_LUMINANCE:       return "LUMINANCE";
    case IL_LUMINANCE_ALPHA: return "LUMINANCE_ALPHA";
    default:                 return "unknown";
    }
  }
  const char* typ2name(ILint typ) {
    switch(typ) {
    case IL_BYTE:           return "BYTE";
    case IL_UNSIGNED_BYTE:  return "UNSIGNED_BYTE";
    case IL_SHORT:          return "SHORT";
    case IL_UNSIGNED_SHORT: return "UNSIGNED_SHORT";
    case IL_INT:            return "INT";
    case IL_UNSIGNED_INT:   return "UNSIGNED_INT";
    case IL_FLOAT:          return "FLOAT";
    case IL_DOUBLE:         return "DOUBLE";
    default:                return "unknown";
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


ClassImp(ZImage);

GMutex ZImage::sILMutex(GMutex::recursive);
Bool_t ZImage::sVerboseLoad(false);

/**************************************************************************/

void ZImage::_init()
{
  // From ZGlass:
  bUseNameStack = false;

  m_data = 0;
  m_bpp  = 0;
  mIL_Name = 0;

  mW = mH = 0;
  mImgFmt = 0;
  mImgTyp = 0;
  mIntFmt = GL_RGB8;

  mSWrap = mTWrap = GL_REPEAT;
  mMagFilter = mMinFilter = GL_NEAREST;
  mEnvMode = GL_DECAL;

  bLoadAdEnlight = false;
  bLoadAdBind    = true;
  bUseShadowing  = true;
  bSmartShadow   = true;
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

  if(load_image()) {
    bLoaded = true;
    if(bUseShadowing && !bSmartShadow)
      shadow();
  }

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

void ZImage::Save(const Text_t* file)
{
  // Save image to file or mFile (if file not provided).
  // Image must be loaded.

  static const Exc_t _eh("ZImage::Save ");

  if(!bLoaded)
    throw(_eh + "image not loaded.");
  if(file == 0) {
    if(mFile.IsNull())
      throw(_eh + "file-name not provided and mFile empty.");
    file = mFile.Data();
  }

  sILMutex.Lock();
  ilBindImage(mIL_Name);
  ilSaveImage(file);
  il_err(_eh.Data());
  sILMutex.Unlock();
}

Bool_t ZImage::IsBindable()
{
  return bLoaded ||
    (bLoadAdBind && mFile != "" &&
     gSystem->AccessPathName(mFile, kReadPermission) == 0);
}

/**************************************************************************/

void ZImage::shadow_check(const Exc_t& eh)
{
  if(IsBindable()) {
    if(bUseShadowing && bSmartShadow) {
      warn_caller(eh + "SmartShadow disbling shadowing on '" + Identify() + "'.");
      bUseShadowing = false;
    }
  } else {
    throw(eh + "'" + Identify() + "' not bindable.");
  }
}

void ZImage::BlurAverage(UInt_t count)
{
  static const Exc_t _eh("ZImage::BlurAverage ");
  shadow_check(_eh);

  sILMutex.Lock();
  bind();
  iluBlurAvg(count);
  unbind();
  sILMutex.Unlock();
  mStampReqTring = Stamp(FID());
}

void ZImage::BlurGaussian(UInt_t count)
{
  static const Exc_t _eh("ZImage::BlurGaussian ");
  shadow_check(_eh);

  sILMutex.Lock();
  bind();
  iluBlurGaussian(count);
  unbind();
  sILMutex.Unlock();
  mStampReqTring = Stamp(FID());
}

void ZImage::Contrastify(Float_t contrast)
{
  static const Exc_t _eh("ZImage::Contrastify ");
  shadow_check(_eh);

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
  static const Exc_t _eh("ZImage::Equalize ");
  shadow_check(_eh);

  sILMutex.Lock();
  bind();
  iluEqualize();
  unbind();
  sILMutex.Unlock();
  mStampReqTring = Stamp(FID());
}

void ZImage::Rotate(Float_t angle)
{
  // Interface to iluRotate (which seems to be badly bugged).

  static const Exc_t _eh("ZImage::Rotate ");
  shadow_check(_eh);

  sILMutex.Lock();
  bind();
  iluRotate(angle);
  unbind();
  sILMutex.Unlock();
  mStampReqTring = Stamp(FID());
}

void ZImage::Mirror()
{
  // Interface to iluMirror (flip around y axis).

  static const Exc_t _eh("ZImage::Mirror ");
  shadow_check(_eh);

  sILMutex.Lock();
  bind();
  iluMirror();
  unbind();
  sILMutex.Unlock();
  mStampReqTring = Stamp(FID());
}

/**************************************************************************/

void ZImage::SetupAsCanvas(Int_t w, Int_t h, Int_t bpp, Bool_t clear_p)
{
  static const Exc_t _eh("ZImage::SetupAsCanvas ");

  switch(bpp) {
  case 1:
    mImgFmt = GL_LUMINANCE;
    mImgTyp = GL_UNSIGNED_BYTE;
    mIntFmt = GL_INTENSITY8;
    break;
  case 2:
    mImgFmt = GL_LUMINANCE;
    mImgTyp = GL_UNSIGNED_SHORT;
    mIntFmt = GL_INTENSITY16;
    break;
  case 3:
    mImgFmt = GL_RGB;
    mImgTyp = GL_UNSIGNED_BYTE;
    mIntFmt = GL_RGB8;
    break;
  case 4:
    mImgFmt = GL_RGBA;
    mImgTyp = GL_UNSIGNED_BYTE;
    mIntFmt = GL_RGBA8;
    break;
  default:
    throw(_eh + "unsupported bytes-per-pixel (1-4 valid).");
  }
  bUseShadowing  = false;

  sILMutex.Lock();
  create_image(w, h, bpp);
  if(clear_p) {
    if(bpp == 1) {
      memset(data(), int(255*mClearColor.gray()), mW*mH);
    }
    else if(bpp == 2) {
      int c = mW*mH;
      unsigned short *p = (unsigned short *) data();
      unsigned short  v = (unsigned short)(65535*mClearColor.gray());
      while(c--) *(p++) = v;
    }
    else {
      ILubyte c[4]; mClearColor.to_ubyte(c);
      ilClearColour(c[0], c[1], c[2], c[3]);
      ilClearImage();
    }
  }
  sILMutex.Unlock();

  mStampReqTring = Stamp(FID());
}

void ZImage::Diagonal(Float_t r, Float_t g, Float_t b)
{
  sILMutex.Lock();
  bind();
  sILMutex.Unlock();

  Int_t max = TMath::Max(mW, mH);
  for(Int_t i=0; i<max; ++i) {
    set_pixel(i, i, r, g, b);
    set_pixel(i, max-i-1, r, g, b);
  }

  mStampReqTring = Stamp(FID());
}

/**************************************************************************/

// User's responsibility to lock/unlock image operations
// and assert that image is loaded (call IsBindable()).

void ZImage::bind()
{
  static const Exc_t _eh("ZImage::bind ");

  if(!bLoaded && bLoadAdBind) {
    if(load_image())
      bLoaded = true;
    else
      goto error;
  }
  else if(bLoaded && bShadowed) {
    if(load_image())
      bShadowed = false;
    else
      goto error;
  }
  else if(bLoaded) {
    ilBindImage(mIL_Name);
    if(il_err("Bind Image"))
      goto error;
  }
  else {
    warn_caller(_eh + "unexpected state for '" + Identify() + "'.");
    goto error;
  }
  _setup();
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
    _reset();
  }
}

bool ZImage::load_image()
{
  ilGenImages(1, &mIL_Name);
  if(il_err("Gen images")) return false;

  ilBindImage(mIL_Name);
  if(il_err("Bind Image")) return false;

  if (!ilLoadImage(const_cast<char *>(mFile.Data()))) {
    il_err(GForm("Load Image <file:%s>", mFile.Data()));
    delete_image();
    return false;
  }

  if(sVerboseLoad) il_id();

  _setup();
  mW = ilGetInteger(IL_IMAGE_WIDTH);
  mH = ilGetInteger(IL_IMAGE_HEIGHT);
  if(ilGetInteger(IL_IMAGE_FORMAT) == IL_COLOR_INDEX) {
    ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
  }
  mImgFmt = gl_format();
  mImgTyp = gl_type();

  return true;
}

void ZImage::create_image(Int_t w, Int_t h, Int_t bpp)
{
  static const Exc_t _eh("ZImage::create_image ");

  delete_image();
  bLoaded = false;
  ilGenImages(1, &mIL_Name);
  if(il_err("Gen Images")) throw(_eh + "Gen Images.");

  ilBindImage(mIL_Name);
  if(il_err("Bind Image")) throw(_eh + "Bind Image.");

  ilTexImage(w, h, 1, bpp, mImgFmt, mImgTyp, 0);
  _setup();
  mW = w; mH = h;
  bLoaded = true;
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

int ZImage::bpp()
{
  return ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);
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
// Low-level pixel control
/**************************************************************************/

void ZImage::set_pixel(Int_t x, Int_t y, Float_t w)
{
  Int_t _w = Int_t(255*w);
  switch(m_bpp) {
  case 1: set_byte(x, y, _w); break;
  case 2: set_short(x, y, _w*255); break;
  case 3: set_rgb(x, y, _w, _w, _w); break;
  case 4: set_rgba(x, y, _w, _w, _w, 1); break;
  }
}

void ZImage::get_pixel(Int_t x, Int_t y, Float_t& w)
{
  switch(m_bpp) {
  case 1: { UChar_t  q; get_byte(x, y, q);  w = q/255.0;   break; }
  case 2: { UShort_t q; get_short(x, y, q); w = q/65535.0; break; }
  case 3: { UChar_t r,g,b;   get_rgb(x, y, r, g, b);     w = (r+g+b)/3.0/255.0; break; }
  case 4: { UChar_t r,g,b,a; get_rgba(x, y, r, g, b, a); w = (r+g+b)/3.0/255.0; break; }
  }
}


void  ZImage::set_pixel(Int_t x, Int_t y, Float_t r, Float_t g, Float_t b)
{
  Int_t _r = Int_t(255*r), _g = Int_t(255*g), _b = Int_t(255*b);
  switch(m_bpp) {
  case 1: set_byte(x, y, (_r+_g+_b)/3); break;
  case 2: set_short(x, y, 255*(_r+_g+_b)/3); break;
  case 3: set_rgb(x, y, _r, _g, _b); break;
  case 4: set_rgba(x, y, _r, _g, _b, 1); break;
  }
}

void  ZImage::get_pixel(Int_t x, Int_t y, Float_t& r, Float_t& g, Float_t& b)
{
  switch(m_bpp) {
  case 1: { UChar_t q; get_byte(x, y, q);   r = g = b = q/255.0;   break; }
  case 2: { UShort_t q; get_short(x, y, q); r = g = b = q/65535.0; break; }
  case 3: { UChar_t _r,_g,_b;    get_rgb(x, y, _r, _g, _b);     r=_r/255.0; g=_g/255.0; b=_b/255.0; break; }
  case 4: { UChar_t _r,_g,_b,_a; get_rgba(x, y, _r, _g, _b, _a); r=_r/255.0; g=_g/255.0; b=_b/255.0; break; }
  }
}

void  ZImage::set_pixel(Int_t x, Int_t y, Float_t r, Float_t g, Float_t b, Float_t a)
{
  Int_t _r = Int_t(255*r), _g = Int_t(255*g), _b = Int_t(255*b), _a = Int_t(255*a);
  switch(m_bpp) {
  case 1: set_byte(x, y, (_r+_g+_b)/3); break;
  case 2: set_short(x, y, 255*(_r+_g+_b)/3); break;
  case 3: set_rgb(x, y, _r, _g, _b); break;
  case 4: set_rgba(x, y, _r, _g, _b, _a); break;
  }
}

void  ZImage::get_pixel(Int_t x, Int_t y, Float_t& r, Float_t& g, Float_t& b, Float_t& a)
{
  a = 1;
  switch(m_bpp) {
  case 1: { UChar_t q; get_byte(x, y, q);   r = g = b = q/255.0;   break; }
  case 2: { UShort_t q; get_short(x, y, q); r = g = b = q/65535.0; break; }
  case 3: { UChar_t _r,_g,_b;    get_rgb(x, y, _r, _g, _b);     r=_r/255.0; g=_g/255.0; b=_b/255.0; break; }
  case 4: { UChar_t _r,_g,_b,_a; get_rgba(x, y, _r, _g, _b, _a); r=_r/255.0; g=_g/255.0; b=_b/255.0; a=_a/255.0; break; }
  }
}

/**************************************************************************/

namespace {
  inline Float_t add_clip(Float_t a, Float_t b, Bool_t clip)
  { a += b; if(clip && a > 1) return 1; return a; }
}

void ZImage::add_pixel(Int_t x, Int_t y, Float_t w, Bool_t clip)
{
  Float_t q; get_pixel(x,y,q);
  set_pixel(x, y, add_clip(w,q,clip));
}

void ZImage::add_pixel(Int_t x, Int_t y, Float_t r, Float_t g, Float_t b, Bool_t clip)
{
  Float_t _r,_g,_b;
  get_pixel(x,y,_r,_g,_b);
  set_pixel(x,y, add_clip(_r,r,clip), add_clip(_g,g,clip), add_clip(_b,b,clip));
}

void ZImage::add_pixel(Int_t x, Int_t y, Float_t r, Float_t g, Float_t b, Float_t a, Bool_t clip)
{
  Float_t _r,_g,_b,_a;
  get_pixel(x,y,_r,_g,_b,_a);
  set_pixel(x,y, add_clip(_r,r,clip), add_clip(_g,g,clip), add_clip(_b,b,clip), add_clip(_a,a,clip));
}

/**************************************************************************/

namespace {
  inline void inc_clip(UChar_t& w, Bool_t clip)
  { if(++w == 0 && clip) --w; }
  inline void inc_clip(UShort_t& w, Bool_t clip)
  { if(++w == 0 && clip) --w; }
}


void ZImage::inc_pixel(Int_t x, Int_t y, Bool_t clip)
{
  switch(m_bpp) {
  case 1: { inc_clip(m_data[y*mH + x], clip); break; }
  case 2: { inc_clip(((UShort_t*)m_data)[y*mH + x], clip); break; }
  case 3:
  case 4: { UChar_t*p = &m_data[m_bpp*(y*mH + x)]; inc_clip(*p++, clip); inc_clip(*p++, clip); inc_clip(*p++, clip); break; }
  }
}

void ZImage::inc_pixel_idx(Int_t idx, Int_t x, Int_t y, Bool_t clip)
{
  switch(m_bpp) {
  case 1: { inc_clip(m_data[y*mH + x], clip); break; }
  case 2: { inc_clip(((UShort_t*)m_data)[y*mH + x], clip); break; }
  case 3:
  case 4: { UChar_t*p = &m_data[m_bpp*(y*mH + x)+idx]; inc_clip(*p++, clip); inc_clip(*p++, clip); inc_clip(*p++, clip); break; }
  }
}


/**************************************************************************/
/**************************************************************************/

// For now, user init is here ... when another piece will need initialization
// it would better be placed somwhere else.

void libGeom1_GLED_user_init()
{
  ilInit();
  iluInit();

  ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
  ilEnable(IL_ORIGIN_SET);
}

void *Geom1_GLED_user_init = (void*)libGeom1_GLED_user_init;

/**************************************************************************/
