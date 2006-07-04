// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_ZImage_H
#define Geom1_ZImage_H

#include <Glasses/ZGlass.h>
#include <Stones/ZColor.h>

#ifndef __CINT__
#include <IL/il.h>
#include <IL/ilu.h>
#else
typedef unsigned int ILuint;
#endif

#include <GL/gl.h>

class ZImage : public ZGlass
{
  // 7777 RnrCtrl(RnrBits(0,4,0,0))
  MAC_RNR_FRIENDS(ZImage);

private:
  void _init();

  void _setup() { m_data = (UChar_t*)data(); m_bpp = bpp(); }
  void _reset() { m_data = 0;      m_bpp = 0; }
  UChar_t      *m_data;		//!
  Int_t         m_bpp;          //!

  ILuint	mIL_Name;	//! X{gp}

protected:

  TString	mFile;		//  X{GS}  7 Filor()
  Int_t		mW;		//  X{G}   7 ValOut(-join=>1)
  Int_t		mH;		//  X{G}   7 ValOut()

  Int_t		mImgFmt;	//  X{GS}  7 PhonyEnum(-const=>1, -join=>1, -vals=>[0,"undef", GL_COLOR_INDEX,col_index, GL_RGB,rgb, GL_RGBA,rgba, GL_BGR,bgr, GL_BGRA,bgra, GL_RED,red, GL_GREEN,green, GL_BLUE,blue, GL_ALPHA,alpha, GL_LUMINANCE,lum, GL_LUMINANCE_ALPHA,lum_alpha, GL_DEPTH_COMPONENT,depth_comp])
  Int_t		mImgTyp;	//  X{GS}  7 PhonyEnum(-const=>1, -join=>1, -vals=>[0,"undef", GL_BYTE,"byte", GL_UNSIGNED_BYTE,"ubyte", GL_SHORT,"short", GL_UNSIGNED_SHORT,"ushort", GL_INT,"int", GL_UNSIGNED_INT,"uint", GL_FLOAT,"float", GL_DOUBLE,"double"])
  Int_t		mIntFmt;	//  X{GS}  7 PhonyEnum(-vals=>[GL_ALPHA4,alpha4, GL_ALPHA8,alpha8, GL_ALPHA12,alpha12, GL_ALPHA16,alpha16, GL_LUMINANCE4,lum4, GL_LUMINANCE8,lum8, GL_LUMINANCE12,lum12, GL_LUMINANCE16,lum16, GL_LUMINANCE4_ALPHA4,lum4_alpha4, GL_LUMINANCE6_ALPHA2,lum6_alpha2, GL_LUMINANCE8_ALPHA8,lum8_alpha8, GL_LUMINANCE12_ALPHA4,lum12_alpha4, GL_LUMINANCE12_ALPHA12,lum12_alpha12, GL_LUMINANCE16_ALPHA16,lum16_alpha16, GL_INTENSITY,"int", GL_INTENSITY4,int4, GL_INTENSITY8,int8, GL_INTENSITY12,int12, GL_INTENSITY16,int16, GL_R3_G3_B2,r3_g3_b2, GL_RGB4,rgb4, GL_RGB5,rgb5, GL_RGB8,rgb8, GL_RGB10,rgb10, GL_RGB12,rgb12, GL_RGB16,rgb16, GL_RGBA2,rgba2, GL_RGBA4,rgba4, GL_RGB5_A1,rgb5_a1, GL_RGBA8,rgba8, GL_RGB10_A2,rgb10_a2, GL_RGBA12,rgba12, GL_RGBA16,rgba16])

  Int_t         mSWrap;         //  X{GST} 7 PhonyEnum(-vals=>[GL_CLAMP,Clamp, GL_CLAMP_TO_EDGE,ClampEdge, GL_REPEAT,Repeat], -width=>8, -join=>1)
  Int_t         mTWrap;         //  X{GST} 7 PhonyEnum(-vals=>[GL_CLAMP,Clamp, GL_CLAMP_TO_EDGE,ClampEdge, GL_REPEAT,Repeat], -width=>8)

  Int_t		mMagFilter;	//  X{GST} 7 PhonyEnum(-vals=>[GL_NEAREST,Nearest, GL_LINEAR,Linear], -width=>8, -join=>1)
  Int_t		mMinFilter;	//  X{GST} 7 PhonyEnum(-vals=>[GL_NEAREST,Nearest, GL_LINEAR,Linear], -width=>8)

  Int_t		mEnvMode;	//  X{GS}  7 PhonyEnum(-vals=>[GL_DECAL,Decal, GL_REPLACE,Replace, GL_MODULATE,Modulate, GL_BLEND,Blend], -width=>8, -join=>1)
  ZColor	mEnvColor;	//  X{PGS} 7 ColorButt()

  Bool_t	bLoadAdEnlight; //  X{GS}  7 Bool(-join=>1)
  Bool_t        bLoadAdBind;    //  X{GS}  7 Bool()
  Bool_t	bUseShadowing;  //  X{GE}  7 Bool(-join=>1)
  Bool_t        bSmartShadow;   //  X{GS}  7 Bool()
  Bool_t	bLoaded;	//! X{G}   7 BoolOut(-join=>1)
  Bool_t	bShadowed;	//! X{G}   7 BoolOut()

  ZColor        mClearColor;    //  X{PGS} 7 ColorButt()

  void shadow_check(const Exc_t& eh);

public:
  ZImage(const Text_t* n="ZImage", const Text_t* t=0) : ZGlass(n,t) { _init(); }
  virtual ~ZImage();

  virtual void AdEnlightenment();

  void SetUseShadowing(Bool_t useshadowing);

  void Load();    // X{E} 7 MButt(-join=>1)
  void Unload();  // X{E} 7 MButt(-join=>1)
  void Save();    // X{E} 7 MButt()

  Bool_t IsBindable();

  void BlurAverage(UInt_t count=1);       // X{E} 7 MCWButt(-join=>1)
  void BlurGaussian(UInt_t count=1);      // X{E} 7 MCWButt()
  void Contrastify(Float_t contrast=1.1); // X{E} 7 MCWButt(-join=>1)
  void Equalize();                        // X{E} 7 MCWButt()
  void Mirror();                          // X{E} 7 MCWButt(-join=>1)
  void Rotate(Float_t angle=90);          // X{E} 7 MCWButt()

  void SetupAsCanvas(Int_t w=256, Int_t h=256,
		     Int_t bpp=1, Bool_t clear_p=true); // X{E} 7 MCWButt()
  void Diagonal(Float_t r=1, Float_t g=1, Float_t b=1); // X{E} 7 MCWButt()

  // Image operations ... locking is user's responsibility
  void  bind();
  void  unbind();
  void	shadow();
  void  delete_image();
  bool  load_image();
  void  create_image(Int_t w, Int_t h, Int_t bpp);

  int	w();
  int	h();
  void* data();
  int   bpp();
  int   gl_format();
  int   gl_type();

  static GMutex sILMutex;
  static Bool_t sVerboseLoad; //! X{GS} 7 Bool()


  //--------------------------------
  // Low-level pixel control.
  //--------------------------------

  void set_byte(Int_t x, Int_t y, UChar_t w)  { m_data[y*mH + x] = w; }
  void get_byte(Int_t x, Int_t y, UChar_t& w) { w = m_data[y*mH + x]; }
  
  void set_short(Int_t x, Int_t y, UShort_t w)  { ((UShort_t*)m_data)[y*mH + x] = w; }
  void get_short(Int_t x, Int_t y, UShort_t& w) { w = ((UShort_t*)m_data)[y*mH + x]; }

  void set_rgb(Int_t x, Int_t y, UChar_t r, UChar_t g, UChar_t b)    { UChar_t*p = &m_data[m_bpp*(y*mH + x)]; *p++=r; *p++=g; *p++=b; }
  void get_rgb(Int_t x, Int_t y, UChar_t& r, UChar_t& g, UChar_t& b) { UChar_t*p = &m_data[m_bpp*(y*mH + x)]; r=*p++; g=*p++; b=*p++; }

  void set_rgba(Int_t x, Int_t y, UChar_t r, UChar_t g, UChar_t b, UChar_t a)     { UChar_t*p = &m_data[m_bpp*(y*mH + x)]; *p++=r; *p++=g; *p++=b; *p++=a; }
  void get_rgba(Int_t x, Int_t y, UChar_t& r, UChar_t& g, UChar_t& b, UChar_t& a) { UChar_t*p = &m_data[m_bpp*(y*mH + x)]; r=*p++; g=*p++; b=*p++; a=*p++; }

  void set_pixel(Int_t x, Int_t y, Float_t w);
  void get_pixel(Int_t x, Int_t y, Float_t& w);
  void set_pixel(Int_t x, Int_t y, Float_t r, Float_t g, Float_t b);
  void get_pixel(Int_t x, Int_t y, Float_t& r, Float_t& g, Float_t& b);
  void set_pixel(Int_t x, Int_t y, Float_t r, Float_t g, Float_t b, Float_t a);
  void get_pixel(Int_t x, Int_t y, Float_t& r, Float_t& g, Float_t& b, Float_t& a);

  void add_pixel(Int_t x, Int_t y, Float_t w, Bool_t clip=true);
  void add_pixel(Int_t x, Int_t y, Float_t r, Float_t g, Float_t b, Bool_t clip=true);
  void add_pixel(Int_t x, Int_t y, Float_t r, Float_t g, Float_t b, Float_t a, Bool_t clip=true);
 
  void inc_pixel(Int_t x, Int_t y, Bool_t clip=true);
  void inc_pixel_idx(Int_t idx, Int_t x, Int_t y, Bool_t clip=true);
 
#include "ZImage.h7"
  ClassDef(ZImage, 1)
}; // endclass ZImage


#endif
