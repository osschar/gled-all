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

class ZImage : public ZGlass {
  // 7777 RnrCtrl(RnrBits(0,4,0,0))
  MAC_RNR_FRIENDS(ZImage);

private:
  void _init();

  ILuint	mIL_Name;	//! X{gp}

protected:

  TString	mFile;		//  X{GS}  7 Filor()
  Int_t		mW;		//  X{G}   7 ValOut(-join=>1)
  Int_t		mH;		//  X{G}   7 ValOut()

  Int_t		mImgFmt;	//  X{GS}  7 PhonyEnum(-const=>1, -join=>1, -vals=>[0,"undef", GL_COLOR_INDEX,col_index, GL_RGB,rgb, GL_RGBA,rgba, GL_BGR,bgr, GL_BGRA,bgra, GL_RED,red, GL_GREEN,green, GL_BLUE,blue, GL_ALPHA,alpha, GL_LUMINANCE,lum, GL_LUMINANCE_ALPHA,lum_alpha, GL_DEPTH_COMPONENT,depth_comp])
  Int_t		mImgTyp;	//  X{GS}  7 PhonyEnum(-const=>1, -join=>1, -vals=>[0,"undef", GL_BYTE,"byte", GL_UNSIGNED_BYTE,"ubyte", GL_SHORT,"short", GL_UNSIGNED_SHORT,"ushort", GL_INT,"int", GL_UNSIGNED_INT,"uint", GL_FLOAT,"float", GL_DOUBLE,"double"])
  Int_t		mIntFmt;	//  X{GS}  7 PhonyEnum(-vals=>[GL_ALPHA4,alpha4, GL_ALPHA8,alpha8, GL_ALPHA12,alpha12, GL_ALPHA16,alpha16, GL_LUMINANCE4,lum4, GL_LUMINANCE8,lum8, GL_LUMINANCE12,lum12, GL_LUMINANCE16,lum16, GL_LUMINANCE4_ALPHA4,lum4_alpha4, GL_LUMINANCE6_ALPHA2,lum6_alpha2, GL_LUMINANCE8_ALPHA8,lum8_alpha8, GL_LUMINANCE12_ALPHA4,lum12_alpha4, GL_LUMINANCE12_ALPHA12,lum12_alpha12, GL_LUMINANCE16_ALPHA16,lum16_alpha16, GL_INTENSITY,"int", GL_INTENSITY4,int4, GL_INTENSITY8,int8, GL_INTENSITY12,int12, GL_INTENSITY16,int16, GL_R3_G3_B2,r3_g3_b2, GL_RGB4,rgb4, GL_RGB5,rgb5, GL_RGB8,rgb8, GL_RGB10,rgb10, GL_RGB12,rgb12, GL_RGB16,rgb16, GL_RGBA2,rgba2, GL_RGBA4,rgba4, GL_RGB5_A1,rgb5_a1, GL_RGBA8,rgba8, GL_RGB10_A2,rgb10_a2, GL_RGBA12,rgba12, GL_RGBA16,rgba16])

  Int_t		mMagFilter;	//  X{GST} 7 PhonyEnum(-vals=>[GL_NEAREST,Nearest, GL_LINEAR,Linear], -width=>8, -join=>1)
  Int_t		mMinFilter;	//  X{GST} 7 PhonyEnum(-vals=>[GL_NEAREST,Nearest, GL_LINEAR,Linear], -width=>8)

  Int_t		mEnvMode;	//  X{GS}  7 PhonyEnum(-vals=>[GL_DECAL,Decal, GL_REPLACE,Replace, GL_MODULATE,Modulate, GL_BLEND,Blend], -width=>8, -join=>1)
  ZColor	mEnvColor;	//  X{PGS} 7 ColorButt()

  Bool_t	bLoadAdEnlight; //  X{GS}  7 Bool(-join=>1)
  Bool_t	bUseShadowing;  //  X{GE}  7 Bool()
  Bool_t	bLoaded;	//! X{G}   7 BoolOut(-join=>1)
  Bool_t	bShadowed;	//! X{G}   7 BoolOut()

public:
  ZImage(const Text_t* n="ZImage", const Text_t* t=0) : ZGlass(n,t) { _init(); }
  virtual ~ZImage();

  virtual void AdEnlightenment();

  void SetUseShadowing(Bool_t useshadowing);

  void Load();    // X{E} 7 MButt(-join=>1)
  void Unload();  // X{E} 7 MButt(-join=>1)
  void Save();    // X{E} 7 MButt()

  void BlurAverage(UInt_t count=1);       // X{E} 7 MCWButt(-join=>1)
  void BlurGaussian(UInt_t count=1);      // X{E} 7 MCWButt()
  void Contrastify(Float_t contrast=1.1); // X{E} 7 MCWButt(-join=>1)
  void Equalize();                        // X{E} 7 MCWButt()

  // Image operations ... locking is user's responsibility
  void  bind();
  void  unbind();
  void	shadow();
  void  delete_image();

  int	w();
  int	h();
  void* data();
  int   gl_format();
  int   gl_type();

  static GMutex sILMutex;
  static Bool_t sVerboseLoad; //! X{GS} 7 Bool()

#include "ZImage.h7"
  ClassDef(ZImage, 1)
    }; // endclass ZImage

GlassIODef(ZImage);

#endif
