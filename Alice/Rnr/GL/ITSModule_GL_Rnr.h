// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_ITSModule_GL_RNR_H
#define Alice_ITSModule_GL_RNR_H

#include <Glasses/ITSModule.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>
#include <RnrBase/RnrDriver.h>
#include <Glasses/ITSDigRnrMod.h>

class ITSModule_GL_Rnr : public ZNode_GL_Rnr
{
 private:
  void _init();

 protected:
  ITSModule*     mITSModule; 
  
  ITSDigRnrMod*	 mDRM;
  RnrModStore    mSegRMS;

 public:
  ITSModule_GL_Rnr(ITSModule* idol) :
    ZNode_GL_Rnr(idol), mITSModule(idol), mSegRMS(ITSDigRnrMod::FID())
  { _init(); }

  virtual void Draw(RnrDriver* rd);
  virtual void Render(RnrDriver* rd);

  void MkCol(Int_t z, Int_t val_min, Int_t val_max);
}; // endclass ITSModule_GL_Rnr


/**************************************************************************/

inline void ITSModule_GL_Rnr::MkCol(Int_t z, Int_t vmin, Int_t vmax)
{
  Float_t c = (z - vmin) * mDRM->mColSep / (vmax - vmin);
  if(c > 1)
    c -= (int)c;
  
  if(mDRM->mRibbon) {
    glColor4fv(mDRM->mRibbon->MarkToCol(c)());
  } else {
    ZColor col( (1 - c)*mDRM->mMinCol + c*mDRM->mMaxCol );
    glColor4fv(col());
  }
}


#endif
