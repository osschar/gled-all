// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_TOFSector_GL_RNR_H
#define Alice_TOFSector_GL_RNR_H

#include <Glasses/TOFSector.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

#include <Glasses/TOFDigRnrMod.h>
#include <RnrBase/RnrDriver.h>

class TOFSector_GL_Rnr : public ZNode_GL_Rnr {
 private:
  void _init();
  void MkCol(Float_t z, Float_t vmin, Float_t vmax);
 protected:
  TOFDigRnrMod*	 mDRM;
  TOFSector*	 mTOFSector;
  RnrModStore    mSegRMS;

 public:
  TOFSector_GL_Rnr(TOFSector* idol) :
    ZNode_GL_Rnr(idol), mTOFSector(idol), mSegRMS(TOFDigRnrMod::FID())
  { _init(); }

  virtual void Draw(RnrDriver* rd);
  virtual void Render(RnrDriver* rd);
  virtual void RenderPlate(Int_t plate, RnrDriver* rd);

}; // endclass TOFSector_GL_Rnr
/**************************************************************************/

inline void TOFSector_GL_Rnr::MkCol(Float_t z, Float_t vmin, Float_t vmax)
{
  Float_t c = (z - vmin) * mDRM->mColSep / (vmax - vmin);
  if(c > 1)
    c -= (int)c;
  
  if(mDRM->mRibbon != 0) {
    glColor4fv(mDRM->mRibbon->MarkToCol(c)());
  } else {
    ZColor col( (1 - c)*mDRM->mMinCol + c*mDRM->mMaxCol );
    glColor4fv(col());
  }
}

#endif
