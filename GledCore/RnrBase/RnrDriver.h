// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_RnrDriver_H
#define GledCore_RnrDriver_H

#include "A_Rnr.h"
#include <GledView/GledViewNS.h>
#include <Stones/ZColor.h>

#include <stack>

namespace GLTextNS {
  class TexFont;
};

/**************************************************************************/
// Rnr driver
/**************************************************************************/

class Lamp_GL_Rnr;

class RnrDriver {
protected:
  Eye*		mEye;		// X{g}

  string	mRnrName;	// X{RGs}
  bool		bUseOwnRnrs;	// X{gs} True for all but GL
  bool		bDryRun;	// X{gs} Don't render, create Rnrs

  int		mMaxDepth;	// X{gs} Max render level
  lpZGlass_t	mPMStack;	// X{g}  Position Matrix Node Stack

  int		mMaxLamps;	// X{g}
  A_Rnr**	mLamps;
  
  bool	             bRnrNames;	 // X{gs}
  GLTextNS::TexFont* mTexFont;   // X{gs}

  int		mWidth;		 // X{gs}
  int		mHeight;	 // X{gs}

  struct RnrMod {
    TObject*        def;
    TObject*        def_autogen;
    stack<TObject*> stack;
    RnrMod() : def(0), def_autogen(0) {}
  };

  typedef hash_map<FID_t, RnrMod>           hRnrMod_t;
  typedef hash_map<FID_t, RnrMod>::iterator hRnrMod_i;

  hRnrMod_t     mRnrMods;
  hRnrMod_i	mRMI;
  FID_t		mRMFid;
  bool          find_rnrmod(FID_t fid);

public:
  RnrDriver(Eye* e, const string& r) : mEye(e), mRnrName(r) {
    bUseOwnRnrs = false; bDryRun = false;
    mMaxDepth = 100;
    mMaxLamps = 8;
    mLamps = new (A_Rnr*)[mMaxLamps];
    mTexFont = 0;
    bRnrNames = false;
  }
  virtual ~RnrDriver() { delete [] mLamps; }

  void FillRnrScheme(RnrScheme* rs, A_Rnr* rnr,
		     const GledViewNS::RnrBits& bits);
  void FillRnrScheme(RnrScheme* rs, OptoStructs::lpZGlassImg_t* imgs,
		     const GledViewNS::RnrBits& bits);

  virtual A_Rnr* GetLensRnr(ZGlass*lens);
  virtual A_Rnr* AssertDefRnr(OptoStructs::ZGlassImg* img);
  virtual A_Rnr* GetRnr(OptoStructs::ZGlassImg* img)
  { return img->fDefRnr ? img->fDefRnr : AssertDefRnr(img); }

  virtual void Render(A_Rnr* img);

  // Interface for Rnrs
  // Position Matrix Name Stack
  void PushPM(ZGlass* g) { mPMStack.push_back(g); }
  void PopPM() 		 { mPMStack.pop_back(); }
  ZGlass* TopPM() 	 { return mPMStack.empty() ? 0 : mPMStack.back(); }
  int  SizePM()          { return mPMStack.size(); }
  void ClearPM()         { mPMStack.clear(); }

  A_Rnr** GetLamps() { return mLamps; }
  void InitLamps();
  int  GetLamp(A_Rnr* l_rnr);
  void ReturnLamp(int lamp);

  //----------------------------------------------------------------

  void     SetDefRnrMod(FID_t fid, TObject* ud);
  TObject* GetDefRnrMod(FID_t fid);
  void     PushRnrMod(FID_t fid, TObject* ud);
  TObject* PopRnrMod(FID_t fid);
  TObject* TopRnrMod(FID_t fid);
  TObject* GetRnrMod(FID_t fid);

  void RemoveRnrModEntry(FID_t fid);
  void CleanUpRnrModDefaults();

#include "RnrDriver.h7"
}; // endclass RnrDriver

/**************************************************************************/

#define RNRDRIVER_GET_RNRMOD(_var_, _rd_, _typ_) \
  _typ_* _var_ = (_typ_*) (_rd_->GetRnrMod(_typ_::FID()))

/**************************************************************************/

inline bool RnrDriver::find_rnrmod(FID_t fid)
{
  if(fid == mRMFid) return true;
  mRMI = mRnrMods.find(fid);
  if(mRMI == mRnrMods.end()) {
    mRMFid.clear(); return false;
  } else {
    mRMFid = fid;   return true;
  }
}

#endif
