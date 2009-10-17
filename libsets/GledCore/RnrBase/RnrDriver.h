// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_RnrDriver_H
#define GledCore_RnrDriver_H

#include "A_Rnr.h"
#include "PMSEntry.h"
#include <Gled/GledNS.h>
#include <Stones/ZTrans.h>
#include <Stones/ZColor.h>

#include <list>
#include <stack>

/**************************************************************************/
// Rnr driver
/**************************************************************************/

class GLRnrDriver;

class RnrDriver : public OptoStructs::ImageConsumer
{
private:
  void _init();

protected:
  Eye*		mEye;		// X{g}

  TString	mRnrName;	// X{RGs}
  Bool_t	bUseOwnRnrs;	// X{gs} True for all but GL

  UInt_t	mRnrCount;	// X{g}
  Bool_t	bDryRun;	// X{gs} Don't render, create Rnrs

  Int_t		mMaxDepth;	// X{gs} Max render level

  //----------------------------------------------------------------
  // PositionMatrix stack & viewing transforms

  list<PMSEntry*> mPMStack;	// X{r}
  PMSEntry        mBotPMSE;

  ZTrans*         mCamFixTrans;  // X{gs} Transformation from camera to camera-base.
  ZTrans*         mCamBaseTrans; // X{gs} CameraBase transformation in abs ref-frame.
  ZTrans*         mCamAbsTrans;  // X{gs} Camera transformation in abs ref-frame.
  ZTrans*         mProjBase;     // X{gs} Projection matrix before cam applied.
  ZTrans*         mProjMatrix;   // X{gs} Projection matrix after cam.

  Int_t	          mWidth;        // X{gs}
  Int_t	          mHeight;       // X{gs}
  Float_t         mZFov;         // X{gs}
  Float_t         mNearClip;     // X{gs}
  Float_t         mFarClip;      // X{gs}

  //----------------------------------------------------------------
  // RnrMod Stack

  struct RMStack
  {
    RnrMod*        m_def;
    RnrMod*        m_def_autogen;
    stack<RnrMod*> m_stack;
    RMStack() : m_def(0), m_def_autogen(0) {}

    RnrMod* get_active()
    {
      if (m_stack.empty())
	return m_def ? m_def : m_def_autogen;
      else
	return m_stack.top();
    }
  };

  typedef hash_map<FID_t, RMStack>           hRMStack_t;
  typedef hash_map<FID_t, RMStack>::iterator hRMStack_i;

  hRMStack_t    mRMStacks;
  hRMStack_i	mRMI;
  FID_t		mRMFid;
  bool          find_rnrmod(FID_t fid);

  //----------------------------------------------------------------
  // Local renderer stash

  typedef hash_map<OptoStructs::ZGlassImg*, A_Rnr*>           hImg2Rnr_t;
  typedef hash_map<OptoStructs::ZGlassImg*, A_Rnr*>::iterator hImg2Rnr_i;

  hImg2Rnr_t    mOwnRnrs;

public:
  RnrDriver(Eye* e, const TString& r);
  virtual ~RnrDriver();

  // ImageConsumer
  virtual void ImageDeath(OptoStructs::ZGlassImg* img);

  void FillRnrScheme(RnrScheme* rs, A_Rnr* rnr,
		     const GledNS::RnrBits& bits);
  void FillRnrScheme(RnrScheme* rs, OptoStructs::lpZGlassImg_t* imgs,
		     const GledNS::RnrBits& bits);

  virtual A_Rnr* GetLensRnr(ZGlass* lens);
  virtual A_Rnr* AssertDefRnr(OptoStructs::ZGlassImg* img);
  virtual A_Rnr* GetRnr(OptoStructs::ZGlassImg* img)
  { return img->fDefRnr ? img->fDefRnr : AssertDefRnr(img); }

  virtual void Render(A_Rnr* img);

  virtual void BeginRender();
  virtual void EndRender();

  // Position Matrix Stack
  void PushPM(PMSEntry& pmse) { pmse.fPrev = mPMStack.back(); mPMStack.push_back(&pmse); }
  void PopPM()                { mPMStack.pop_back(); }

  PMSEntry& TopPM() 	{ return *mPMStack.back(); }
  ZNode*    TopPMNode() { return  mPMStack.back()->fNode; }

  int  SizePM()  { return mPMStack.size(); }
  void ClearPM() { mPMStack.clear(); }

  ZTrans& ToGCS()   { return mPMStack.back()->ToGCS(); }
  ZTrans& FromGCS() { return mPMStack.back()->FromGCS(); }

  //----------------------------------------------------------------

  void    SetDefRnrMod(FID_t fid, RnrMod* ud);
  RnrMod* GetDefRnrMod(FID_t fid);
  void    PushRnrMod(FID_t fid, RnrMod* ud);
  RnrMod* PopRnrMod(FID_t fid);
  RnrMod* TopRnrMod(FID_t fid);
  RnrMod* GetRnrMod(FID_t fid);

  void RemoveRnrModEntry(FID_t fid);
  void CleanUpRnrModDefaults();

  //----------------------------------------------------------------

  GLRnrDriver* GL() { return (GLRnrDriver*)this; }

#include "RnrDriver.h7"
}; // endclass RnrDriver


/**************************************************************************/
// Inlines
/**************************************************************************/

inline bool RnrDriver::find_rnrmod(FID_t fid)
{
  if (fid == mRMFid) return true;
  mRMI = mRMStacks.find(fid);
  if (mRMI == mRMStacks.end())
  {
    mRMFid.clear();
    return false;
  }
  else
  {
    mRMFid = fid;
    return true;
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
