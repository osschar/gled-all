// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_WGlDirectory_H
#define GledCore_WGlDirectory_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>
#include <Gled/GledNS.h>

class WGlDirectory : public ZNode
{
  MAC_RNR_FRIENDS(WGlDirectory);

 public:
  enum StepMode_e { SM_XYZ, SM_YXZ, SM_XZY };

 private:
  void _init();

 protected:
  ZLink<AList>      mContents;     //  X{GE} L{}
  ZLink<ZList>      mSelection;    //  X{GS} L{}
  ZLink<ZGlass>     mLastClicked;  //! X{GS} L{}

  StepMode_e        mStepMode;     // X{GS} 7 PhonyEnum()
  Float_t           mDx;           // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mDy;           // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mDz;           // X{GS} 7 Value(-range=>[-1000,1000,1,1000])

  Int_t             mNx;           // X{GS} 7 Value(-range=>[1,1000,1], -join=>1)
  Int_t             mNy;           // X{GS} 7 Value(-range=>[1,1000,1], -join=>1)
  Int_t             mNz;           // X{GS} 7 Value(-range=>[1,1000,1])

  Int_t             mNEntries;      //!
  TimeStamp_t       mNEntriesStamp; //!
  Int_t             count_entries();

  Int_t             mFirst;        // X{GE} 7 Value(-range=>[0,100,1])
  Bool_t            bDrawBox;      // X{GS} 7 Bool()
  Float_t           mBoxOx;        // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mBoxOy;        // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t           mBoxOz;        // X{GS} 7 Value(-range=>[-1000,1000,1,1000])
  Float_t           mBoxDx;        // X{GS} 7 Value(-range=>[0,1000,1,1000], -join=>1)
  Float_t           mBoxDy;        // X{GS} 7 Value(-range=>[0,1000,1,1000], -join=>1)
  Float_t           mBoxDz;        // X{GS} 7 Value(-range=>[0,1000,1,1000])

  Bool_t            bDrawText;     // X{GS} 7 Bool(-join=>1)
  Bool_t            bDrawTitle;    // X{GS} 7 Bool(-join=>1)
  Float_t           mNameFraction; // X{GS} 7 Value(-range=>[0,1,1,1000])
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

  Bool_t            bDrawPageCtrl;  // X{GS} 7 Bool()
  ZColor            mPageColor;     // X{GSP} 7 ColorButt(-join=>1)
  ZColor            mSymColor;      // X{GSP} 7 ColorButt()
  Float_t           mPageCtrlWidth;           // X{GS} 7 Value(-range=>[0,1,1,1000],-join=>1)
  Float_t           mPageInfoWidth;           // X{GS} 7 Value(-range=>[0,1,1,1000])
  Float_t           mPageCtrlOff;         // X{GS} 7 Value(-range=>[0,1,1,1000],-join=>1)
  Float_t           mPageInfoOff;         // X{GS} 7 Value(-range=>[0,1,1,1000])

  ZLink<ZGlass>       mCbackAlpha;         //  X{GS} L{}
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
  { mCbackMethodInfo = 0; mCbackBetaClassInfo = 0; mNEntriesStamp = 0; }

  void SetContents(AList* cont);

  void SetFirst(Int_t first) {
    Int_t first_full_page = count_entries() - mNx*mNy*mNz + 1;
    if(first > first_full_page) first = first_full_page;
    if(first < 1) first = 1;
    mFirst = first;
    Stamp(FID());
  }

  void Prev()     { SetFirst(mFirst - 1); }           // X{E}
  void PrevPage() { SetFirst(mFirst - mNx*mNy*mNz); } // X{E}
  void Next()     { SetFirst(mFirst + 1); }           // X{E}
  void NextPage() { SetFirst(mFirst + mNx*mNy*mNz); } // X{E}

  void StandardPersp();  //  X{E} 7 MButt(-join=>1)
  void StandardFixed();  //  X{E} 7 MButt(-join=>1)
  void StandardPixel();  //  X{E} 7 MButt()

#include "WGlDirectory.h7"
  ClassDef(WGlDirectory, 1);
}; // endclass WGlDirectory


#endif
