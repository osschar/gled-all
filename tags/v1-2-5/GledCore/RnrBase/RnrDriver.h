// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_RnrDriver_H
#define GledCore_RnrDriver_H

#include "A_Rnr.h"
#include "PMSEntry.h"
#include <GledView/GledViewNS.h>
#include <Stones/ZTrans.h>
#include <Stones/ZColor.h>

#include <list>
#include <stack>

namespace GLTextNS {
  class TexFont;
};


/**************************************************************************/
// Rnr driver
/**************************************************************************/

class Lamp_GL_Rnr;

class RnrDriver : public OptoStructs::ImageConsumer {
private:
  void _init();

protected:
  Eye*		mEye;		// X{g}

  string	mRnrName;	// X{RGs}
  Bool_t	bUseOwnRnrs;	// X{gs} True for all but GL

  UInt_t	mRnrCount;	// X{g}
  Bool_t	bDryRun;	// X{gs} Don't render, create Rnrs

  Int_t		mMaxDepth;	// X{gs} Max render level

  // --------------------------------

protected:
  list<PMSEntry*> mPMStack;	// X{r}
  PMSEntry        mBotPMSE;
  ZTrans*         mAbsCamera;   // X{gs}

  // --------------------------------

  Int_t		mMaxLamps;	// X{g}
  A_Rnr**	mLamps;

  Int_t		mMaxClipPlanes; // X{g}
  A_Rnr**	mClipPlanes;

  Bool_t        bMarkupNodes;	 // X{gs}

  Int_t		mWidth;		 // X{gs}
  Int_t		mHeight;	 // X{gs}

  struct RMStack {
    RnrMod*        def;
    RnrMod*        def_autogen;
    stack<RnrMod*> stack;
    RMStack() : def(0), def_autogen(0) {}
  };

  typedef hash_map<FID_t, RMStack>           hRMStack_t;
  typedef hash_map<FID_t, RMStack>::iterator hRMStack_i;

  hRMStack_t    mRMStacks;
  hRMStack_i	mRMI;
  FID_t		mRMFid;
  bool          find_rnrmod(FID_t fid);

  typedef hash_map<OptoStructs::ZGlassImg*, A_Rnr*>           hImg2Rnr_t;
  typedef hash_map<OptoStructs::ZGlassImg*, A_Rnr*>::iterator hImg2Rnr_i;
  
  hImg2Rnr_t    mOwnRnrs;

public:
  RnrDriver(Eye* e, const string& r);
  virtual ~RnrDriver();

  virtual void ImageDeath(OptoStructs::ZGlassImg* img);

  void FillRnrScheme(RnrScheme* rs, A_Rnr* rnr,
		     const GledViewNS::RnrBits& bits);
  void FillRnrScheme(RnrScheme* rs, OptoStructs::lpZGlassImg_t* imgs,
		     const GledViewNS::RnrBits& bits);

  virtual A_Rnr* GetLensRnr(ZGlass*lens);
  virtual A_Rnr* AssertDefRnr(OptoStructs::ZGlassImg* img);
  virtual A_Rnr* GetRnr(OptoStructs::ZGlassImg* img)
  { return img->fDefRnr ? img->fDefRnr : AssertDefRnr(img); }

  virtual void Render(A_Rnr* img);

  virtual void BeginRender();
  virtual void EndRender();

  // Position Matrix Stack
  void PushPM(PMSEntry& pmse)
  { pmse.fPrev = mPMStack.back(); mPMStack.push_back(&pmse); }
  void PopPM()
  { mPMStack.pop_back(); }

  PMSEntry& TopPM() 	{ return *mPMStack.back(); }
  ZNode*    TopPMNode() { return mPMStack.back()->fNode; }

  int  SizePM()  { return mPMStack.size(); }
  void ClearPM() { mPMStack.clear(); }

  ZTrans& ToGCS()   { return mPMStack.back()->ToGCS(); }
  ZTrans& FromGCS() { return mPMStack.back()->FromGCS(); }

  // Lamps
  A_Rnr** GetLamps() { return mLamps; }
  Int_t   GetLamp(A_Rnr* rnr);
  void    ReturnLamp(Int_t lamp);

  // Clipping planes
  A_Rnr** GetClipPlanes() { return mClipPlanes; }
  Int_t   GetClipPlane(A_Rnr* rnr);
  void    ReturnClipPlane(Int_t lamp);

  //----------------------------------------------------------------

  void    SetDefRnrMod(FID_t fid, RnrMod* ud);
  RnrMod* GetDefRnrMod(FID_t fid);
  void    PushRnrMod(FID_t fid, RnrMod* ud);
  RnrMod* PopRnrMod(FID_t fid);
  RnrMod* TopRnrMod(FID_t fid);
  RnrMod* GetRnrMod(FID_t fid);

  void RemoveRnrModEntry(FID_t fid);
  void CleanUpRnrModDefaults();

#include "RnrDriver.h7"
}; // endclass RnrDriver

/**************************************************************************/

inline bool RnrDriver::find_rnrmod(FID_t fid)
{
  if(fid == mRMFid) return true;
  mRMI = mRMStacks.find(fid);
  if(mRMI == mRMStacks.end()) {
    mRMFid.clear(); return false;
  } else {
    mRMFid = fid;   return true;
  }
}

/**************************************************************************/
// Preprocessor shortcuts for render-mod access.
/**************************************************************************/

#define RNRDRIVER_GET_RNRMOD_BOTH(_var_, _rd_, _typ_) \
  RnrMod* _var_ = _rd_->GetRnrMod(_typ_::FID()); \
  _typ_* _var_ ## _lens = (_typ_*) _var_->fLens; \
  _typ_ ## _GL_Rnr* _var_ ## _rnr = (_typ_ ## _GL_Rnr*) _var_->fRnr

#define RNRDRIVER_GET_RNRMOD_LENS(_var_, _rd_, _typ_) \
  RnrMod* _var_ = _rd_->GetRnrMod(_typ_::FID()); \
  _typ_* _var_ ## _lens = (_typ_*) _var_->fLens

#define RNRDRIVER_GET_RNRMOD_RNR(_var_, _rd_, _typ_) \
  RnrMod* _var_ = _rd_->GetRnrMod(_typ_::FID()); \
  _typ_ ## _GL_Rnr* _var_ ## _rnr = (_typ_ ## _GL_Rnr*) _var_->fRnr

#define RNRDRIVER_GET_RNRMOD(_var_, _rd_, _typ_) \
  RnrMod* _var_ = _rd_->GetRnrMod(_typ_::FID())


#define RNRDRIVER_CAST_RNRMOD_BOTH(_var_, _rnrmod_, _typ_) \
  _typ_* _var_ ## _lens = (_typ_*) _rnrmod_->fLens; \
  _typ_ ## _GL_Rnr* _var_ ## _rnr = (_typ_ ## _GL_Rnr*) _rnrmod_->fRnr

#define RNRDRIVER_CAST_RNRMOD_LENS(_var_, _rnrmod_, _typ_) \
  _typ_* _var_ ## _lens = (_typ_*) _rnrmod_->fLens

#define RNRDRIVER_CAST_RNRMOD_RNR(_var_, _rnrmod_, _typ_) \
  _typ_ ## _GL_Rnr* _var_ ## _rnr = (_typ_ ## _GL_Rnr*) _rnrmod_->fRnr


#endif