// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_RnrDriver
#define Gled_RnrDriver

#include "A_Rnr.h"
#include "RnrScheme.h"
#include <Eye/OptoStructs.h>
#include <GledView/GledViewNS.h>
#include <GledView/GLTextNS.h>
#include <Stones/ZColor.h>

#include <stack>

/**************************************************************************/
// Rnr driver
/**************************************************************************/

class Lamp_GL_Rnr;

class RnrDriver {
protected:
  Eye*		mEye;		// X{G}

  string	mRnrName;	// X{RGs}
  bool		bUseOwnRnrs;	// X{Gs} True for all but GL
  bool		bDryRun;	// X{Gs} Don't render, create Rnrs

  int		mMaxDepth;	// X{Gs} Max render level
  lpZGlass_t	mPMStack;	// X{G}  Position Matrix Node Stack

  int		mMaxLamps;	// X{G}
  A_Rnr**	mLamps;
  
  bool		bRnrPureGlasses; // X{Gs}
  bool		bRnrNames;	 // X{Gs}
  bool		bRnrTiles;	 // X{Gs}
  bool		bRnrFrames;	 // X{Gs}
  Float_t	mNameOffset;	 // X{Gs}

  Int_t		mTextSize;	 // X{Gs}
  ZColor	mTextCol;	 // X{pr}
  ZColor	mTileCol;	 // X{pr}
  string	mTilePos;	 // X{rGs}

  int		mWidth;		 // X{Gs}
  int		mHeight;	 // X{Gs}

  void fill_rnrelem_vec(OptoStructs::A_GlassView* gv,
			const GledViewNS::RnrBits& bits,
			vlRnrElement_t& rev,
			bool as_list, bool full_descent);

public:
  GLTextNS::TexFont* fTexFont;

  RnrDriver(Eye* e, const string& r) : mEye(e), mRnrName(r) {
    bUseOwnRnrs = false; bDryRun = false;
    mMaxDepth = 100;
    mMaxLamps = 8;
    mLamps = new A_Rnr*[mMaxLamps];
    bRnrPureGlasses = false;
    bRnrNames = false; bRnrTiles = false;
    fTexFont = 0;
  }
  virtual ~RnrDriver() { delete [] mLamps; }

  virtual void AssertGlassRnr(OptoStructs::A_GlassView* gv);
  virtual void AssertListRnrs(OptoStructs::A_GlassView* lv);

  virtual void PrepareRnrElements(OptoStructs::A_GlassView* gv,
				  vlRnrElement_t& rev);

  virtual void Render(OptoStructs::A_GlassView* gv);

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

  A_Rnr* GetDefRnr(ZGlass* g);

#include "RnrDriver.h7"
}; // endclass RnrDriver

#endif
