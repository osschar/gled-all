// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
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
  // 7777 RnrCtrl("false, 3, RnrBits(1,2,3,0, 0,0,0,0)")
  MAC_RNR_FRIENDS(ZImage);

private:
  void _init();

protected:
  TimeStamp_t	mStampReqTexture; //! TimeStamp of last change of image
  ILuint	mIL_Name;	  //! X{gp}

  TString	mFile;		//  X{GS}  7 Filor()
  Int_t		mW;		//  X{G}   7 ValOut(-join=>1)
  Int_t		mH;		//  X{G}   7 ValOut()

  Int_t		mMagFilter;	//  X{GE}  7 PhonyEnum(-vals=>[GL_NEAREST,Nearest, GL_LINEAR,Linear], -width=>8, -join=>1)
  Int_t		mMinFilter;	//  X{GE}  7 PhonyEnum(-vals=>[GL_NEAREST,Nearest, GL_LINEAR,Linear], -width=>8)

  Int_t		mEnvMode;	//  X{GS}  7 PhonyEnum(-vals=>[GL_DECAL,Decal, GL_REPLACE,Replace, GL_MODULATE,Modulate, GL_BLEND,Blend], -width=>8, -join=>1)
  ZColor	mEnvColor;	//  X{PGS} 7 ColorButt()

  Bool_t	bLoaded;	//  X{G}   7 BoolOut(-join=>1)
  Bool_t	bLoadAdEnlight; //  X{GS}  7 Bool()

public:
  ZImage(const Text_t* n="ZImage", const Text_t* t=0) : ZGlass(n,t) { _init(); }
  virtual ~ZImage();

  virtual void AdEnlightenment();

  virtual void SetStamps(TimeStamp_t s)
  { ZGlass::SetStamps(s); mStampReqTexture = s; }

  void SetMagFilter(Int_t magfilter) {
    WriteLock(); mMagFilter = magfilter;
    mStampReqTexture = Stamp(LibID(), ClassID()); WriteUnlock();
  }
  void SetMinFilter(Int_t minfilter) {
    WriteLock(); mMinFilter = minfilter; 
    mStampReqTexture = Stamp(LibID(), ClassID()); WriteUnlock();
  }

  void Load();   // X{E} 7 MButt(-join=>1)
  void Unload(); // X{E} 7 MButt(-join=>1)
  void Save();   // X{E} 7 MButt()

  // Image operations ... locking is user's responsibility
  void  bind();
  void  delete_image();
  int	w();
  int	h();
  void* data();
  int   gl_format();
  int   gl_type();

  static GMutex sILMutex;

#include "ZImage.h7"
  ClassDef(ZImage, 1)
}; // endclass ZImage

GlassIODef(ZImage);

#endif
