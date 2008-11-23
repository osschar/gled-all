// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_WGlValuator_H
#define GledCore_WGlValuator_H

#include <Glasses/WGlWidget.h>
#include <Gled/GledNS.h>

class TRealData;

class WGlValuator : public WGlWidget
{
  MAC_RNR_FRIENDS(WGlValuator);

private:
  void _init();

protected:
  Double_t        mMin;    // X{GS} 7 Value(-join=>1)
  Double_t        mMax;    // X{GS} 7 Value()
  Int_t           mStepA;  // X{GS} 7 Value(-join=>1)
  Int_t           mStepB;  // X{GS} 7 Value()

  TString         mFormat; // X{GS} 7 Textor()

  Bool_t          bConstVal;  // X{GS} 7 Bool()

  TString         mCbackMemberName; // X{GS} Ray{CbackReset} 7 Textor()

  // Local stash
  GledNS::DataMemberInfo* mDataMemberInfo; //!
  TDataMember*            mDataMember;     //!

public:
  WGlValuator(const Text_t* n="WGlValuator", const Text_t* t=0) :
    WGlWidget(n,t) { _init(); }

  virtual void EmitCbackResetRay() { mDataMemberInfo = 0; mDataMember = 0; }

  GledNS::DataMemberInfo* GetDataMemberInfo();
  TDataMember*            GetDataMember();

  Bool_t DataOK();

#include "WGlValuator.h7"
  ClassDef(WGlValuator, 1);
}; // endclass WGlValuator

#endif
