// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZNodeListLink_H
#define GledCore_ZNodeListLink_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>
#include <Gled/GledNS.h>

class ZNodeListLink : public ZNode {
  MAC_RNR_FRIENDS(ZNodeListLink);

public:
  enum ViewMode_e { VM_Nop, VM_OrthoPixel, VM_OrthoFixed };
  enum StepMode_e { SM_XYZ, SM_YXZ, SM_Circle, SM_Sphere };

private:
  void _init();

protected:
  ZList*            mContents;     // X{GS} L{}
  ZGlass*           mCurrent;      // X{GS} L{}
  ZList*            mSelection;    // X{GS} L{}

  ViewMode_e        mViewMode;     // X{GS} 7 PhonyEnum()
  Float_t           mOrthoW;       // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mOrthoH;       // X{GS} 7 Value(-range=>[-1000,1000,1,1000])
  Float_t           mOrthoNear;    // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mOrthoFar;     // X{GS} 7 Value(-range=>[-1000,1000,1,1000])

  StepMode_e        mStepMode;     // X{GS} 7 PhonyEnum()
  Float_t           mOx;           // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mOy;           // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mOz;           // X{GS} 7 Value(-range=>[-1000,1000,1,1000])

  Float_t           mDx;           // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mDy;           // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mDz;           // X{GS} 7 Value(-range=>[-1000,1000,1,1000])

  Int_t             mNx;           // X{GS} 7 Value(-range=>[0,1000,1], -join=>1)
  Int_t             mNy;           // X{GS} 7 Value(-range=>[0,1000,1], -join=>1)
  Int_t             mNz;           // X{GS} 7 Value(-range=>[0,1000,1])


  Bool_t            bDrawBox;      // X{GS} 7 Bool()
  Float_t           mBoxDx;        // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mBoxDy;        // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mBoxDz;        // X{GS} 7 Value(-range=>[-1000,1000,1,1000])       
  Float_t           mBoxSx;        // X{GS} 7 Value(-range=>[0,1000,1,1000], -join=>1)
  Float_t           mBoxSy;        // X{GS} 7 Value(-range=>[0,1000,1,1000], -join=>1)
  Float_t           mBoxSz;        // X{GS} 7 Value(-range=>[0,1000,1,1000])       

  Bool_t            bDrawText;     // X{GS} 7 Bool()
  Float_t           mTextYSize;    // X{GS} 7 Value(-range=>[0,1000,1,1000], -join=>1)
  Bool_t            bTextCenter;   // X{GS} 7 Bool()
  Float_t           mTextMaxLen;   // X{GS} 7 Value(-range=>[0,1000,1,1000], -join=>1)
  Float_t           mTextFadeW;    // X{GS} 7 Value(-range=>[0,1000,1,1000])
  Float_t           mTextDx;       // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mTextDy;       // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mTextDz;       // X{GS} 7 Value(-range=>[-1000,1000,1,1000])       
  Float_t           mTextA1;       // X{GS} 7 Value(-range=>[-360,360,1,1000], -join=>1)
  Float_t           mTextA2;       // X{GS} 7 Value(-range=>[-360,360,1,1000], -join=>1)
  Float_t           mTextA3;       // X{GS} 7 Value(-range=>[-360,360,1,1000])       

  Bool_t            bDrawTile;     // X{GS} 7 Bool(-join=>1)
  Bool_t            bFullTile;     // X{GS} 7 Bool(-join=>1)
  Bool_t            bDrawFrame;    // X{GS} 7 Bool()
  Float_t           mXBorder;      // X{GS} 7 Value(-range=>[-100,100,1,1000], -join=>1) 
  Float_t           mYBorder;      // X{GS} 7 Value(-range=>[-100,100,1,1000]) 

  ZColor            mTextColor;    // X{GSP} 7 ColorButt(-join=>1)
  ZColor            mBoxColor;     // X{GSP} 7 ColorButt()
  ZColor            mSelColor;     // X{GSP} 7 ColorButt(-join=>1)
  ZColor            mCurColMod;    // X{GSP} 7 ColorButt()

  ZGlass*             mCbackAlpha;      //  X{GS} L{}
  TString             mCbackMethodName; //  X{GS} Ray{CbackReset} 7 Textor()
  GledNS::MethodInfo* mCbackMethodInfo; //!

public:
  ZNodeListLink(const Text_t* n="ZNodeListLink", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  GledNS::MethodInfo* GetCbackMethodInfo();

  void EmitCbackResetRay() { mCbackMethodInfo = 0; }

  void XPrint();         //! X{E} 7 MButt()

  void StandardPersp();  //  X{E} 7 MButt(-join=>1)
  void StandardFixed();  //  X{E} 7 MButt(-join=>1)
  void StandardPixel();  //  X{E} 7 MButt()

#include "ZNodeListLink.h7"
  ClassDef(ZNodeListLink, 1)
}; // endclass ZNodeListLink

GlassIODef(ZNodeListLink);

#endif
