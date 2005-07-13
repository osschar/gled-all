// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_WGlDirectory_H
#define GledCore_WGlDirectory_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>
#include <Gled/GledNS.h>

class WGlDirectory : public ZNode {
  MAC_RNR_FRIENDS(WGlDirectory);

public:
  enum StepMode_e { SM_XYZ, SM_YXZ, SM_XZY };

private:
  void _init();

protected:
  ZList*            mContents;     // X{GS} L{}
  ZList*            mSelection;    // X{GS} L{}

  StepMode_e        mStepMode;     // X{GS} 7 PhonyEnum()
  Float_t           mDx;           // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mDy;           // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mDz;           // X{GS} 7 Value(-range=>[-1000,1000,1,1000])

  Int_t             mNx;           // X{GS} 7 Value(-range=>[0,1000,1], -join=>1)
  Int_t             mNy;           // X{GS} 7 Value(-range=>[0,1000,1], -join=>1)
  Int_t             mNz;           // X{GS} 7 Value(-range=>[0,1000,1])

  Bool_t            bDrawBox;      // X{GS} 7 Bool()
  Float_t           mBoxOx;        // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mBoxOy;        // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mBoxOz;        // X{GS} 7 Value(-range=>[-1000,1000,1,1000])       
  Float_t           mBoxDx;        // X{GS} 7 Value(-range=>[0,1000,1,1000], -join=>1)
  Float_t           mBoxDy;        // X{GS} 7 Value(-range=>[0,1000,1,1000], -join=>1)
  Float_t           mBoxDz;        // X{GS} 7 Value(-range=>[0,1000,1,1000])       

  Bool_t            bDrawText;     // X{GS} 7 Bool()
  Float_t           mTextOx;       // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mTextOy;       // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mTextOz;       // X{GS} 7 Value(-range=>[-1000,1000,1,1000])       
  Float_t           mTextDx;       // X{GS} 7 Value(-range=>[0,1000,1,1000], -join=>1)
  Float_t           mTextDy;       // X{GS} 7 Value(-range=>[0,1000,1,1000])
  Float_t           mTextA1;       // X{GS} 7 Value(-range=>[-360,360,1,1000], -join=>1)
  Float_t           mTextA2;       // X{GS} 7 Value(-range=>[-360,360,1,1000], -join=>1)
  Float_t           mTextA3;       // X{GS} 7 Value(-range=>[-360,360,1,1000])       

  ZColor            mBoxColor;     // X{GSP} 7 ColorButt(-join=>1)
  ZColor            mSelColor;     // X{GSP} 7 ColorButt()

  ZGlass*             mCbackAlpha;         //  X{GS} L{}
  TString             mCbackMethodName;    //  X{GS} Ray{CbackReset} 7 Textor()
  TString             mCbackBetaType;      //  X{GS} Ray{CbackReset} 7 Textor()
  GledNS::MethodInfo* mCbackMethodInfo;    //!
  GledNS::ClassInfo*  mCbackBetaClassInfo; //!

public:
  WGlDirectory(const Text_t* n="WGlDirectory", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  GledNS::MethodInfo* GetCbackMethodInfo();
  GledNS::ClassInfo*  GetCbackBetaClassInfo();
  void EmitCbackResetRay()
  { mCbackMethodInfo = 0; mCbackBetaClassInfo = 0; }

  void StandardPersp();  //  X{E} 7 MButt(-join=>1)
  void StandardFixed();  //  X{E} 7 MButt(-join=>1)
  void StandardPixel();  //  X{E} 7 MButt()

#include "WGlDirectory.h7"
  ClassDef(WGlDirectory, 1)
}; // endclass WGlDirectory

GlassIODef(WGlDirectory);

#endif
