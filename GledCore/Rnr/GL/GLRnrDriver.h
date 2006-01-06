// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GLRnrDriver_H
#define GledCore_GLRnrDriver_H

#include <RnrBase/RnrDriver.h>

class ZGlColorFader;

class GLRnrDriver : public RnrDriver
{
protected:
  Bool_t                    bInPicking;  // X{G}
  Bool_t                    bDoPickOps;  // X{gs}

  UInt_t                    mPickCount;  // X{g}
  UInt_t                    mPickSize;
  A_Rnr::vNSE_t             mPickVector; // X{r}
  list<UInt_t>              mStackCopy;

  void push_name(A_Rnr* rnr, void* ud);
  void pop_name();

  A_Rnr*                    mBelowMouse; // X{gs}
  A_Rnr*                    mPushed;     // X{gs}
  A_Rnr*                    mFocus;      // X{gs}

  //--------------------------------

  Int_t		mMaxLamps;	 // X{g}
  A_Rnr**	mLamps;

  Int_t		mMaxClipPlanes;  // X{g}
  A_Rnr**	mClipPlanes;

  Bool_t        bInDLRebuild;    // X{gs}
  Bool_t        bMarkupNodes;	 // X{gs}

  Bool_t        bRedraw;         // X{gs}

  RMStack*      mFaderStack;

public:
  GLRnrDriver(Eye* e, const TString& r);
  virtual ~GLRnrDriver();

  virtual void BeginRender();
  virtual void EndRender();

  // NameStack
  virtual void BeginPick();
  virtual void EndPick();
  
  A_Rnr::NSE_t& NameStack(UInt_t i)
  { if(i>mPickCount) i=0; return mPickVector[i]; }

  Bool_t PickingP()                     { return bInPicking && bDoPickOps; }
  void PushName(A_Rnr* rnr, void* ud=0) { if(PickingP()) push_name(rnr, ud); }
  void PopName()                        { if(PickingP()) pop_name(); }

  void ClearNameStack();
  void RestoreNameStack();

  // Lamps
  A_Rnr** GetLamps() { return mLamps; }
  Int_t   GetLamp(A_Rnr* rnr);
  void    ReturnLamp(Int_t lamp);

  // Clipping planes
  A_Rnr** GetClipPlanes() { return mClipPlanes; }
  Int_t   GetClipPlane(A_Rnr* rnr);
  void    ReturnClipPlane(Int_t lamp);

  // Color, PointSize and LineWidth scaling
  Float_t Color(Float_t r, Float_t g, Float_t b, Float_t a=1);
  Float_t Color(const ZColor& c) { return Color(c.r(), c.g(), c.b(), c.a()); }
  Float_t PointSize(Float_t size);
  Float_t LineWidth(Float_t width);

  ZGlColorFader& ColorFader()
  { return (ZGlColorFader&) *mFaderStack->get_active()->fLens; }

#include "GLRnrDriver.h7"
}; // endclass GLRnrDriver

#endif
