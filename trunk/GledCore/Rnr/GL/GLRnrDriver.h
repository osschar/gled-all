// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GLRnrDriver_H
#define GledCore_GLRnrDriver_H

#include <RnrBase/RnrDriver.h>

class GLRnrDriver : public RnrDriver
{
private:

protected:
  Int_t		mMaxLamps;	// X{g}
  A_Rnr**	mLamps;

  Int_t		mMaxClipPlanes; // X{g}
  A_Rnr**	mClipPlanes;

  Bool_t        bInDLRebuild;    // X{gs}
  Bool_t        bMarkupNodes;	 // X{gs}

public:
  GLRnrDriver(Eye* e, const string& r);
  virtual ~GLRnrDriver();

  virtual void BeginRender();
  virtual void EndRender();

  // Lamps
  A_Rnr** GetLamps() { return mLamps; }
  Int_t   GetLamp(A_Rnr* rnr);
  void    ReturnLamp(Int_t lamp);

  // Clipping planes
  A_Rnr** GetClipPlanes() { return mClipPlanes; }
  Int_t   GetClipPlane(A_Rnr* rnr);
  void    ReturnClipPlane(Int_t lamp);

#include "GLRnrDriver.h7"
}; // endclass GLRnrDriver

#endif
